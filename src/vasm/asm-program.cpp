/* 
 * File:   asm_program.cpp
 * Author: ben
 * 
 * Created on 5 agosto 2010, 21.05
 */

#include "asm-program.h"

#include "IncludesTree.h"
#include "../FileHandler.h"
#include "disassembler/Disassembler.h"

#include <sstream>
using namespace std;

void
asm_program::checkInstructions(const bool & usingTemps) const
{
  bool error = false;
  for(deque<asm_function *>::const_iterator iter = functions.begin();
      iter != functions.end(); iter++)
  {
    asm_function * func = (*iter);
    for(ListOfStmts::const_iterator stmt_it = func->stmts.begin();
        stmt_it != func->stmts.end(); stmt_it++)
    {
      const asm_statement * stmt = *stmt_it;
      if (stmt->isInstruction()) {
        try {
          ((const asm_instruction_statement *)stmt)->checkArgs();

          if (usingTemps && stmt->getType() == ASM_FUNCTION_CALL) {
            const asm_function_call * f_stmt = (const asm_function_call *) stmt;

            const vector<asm_arg *> & args = f_stmt->args;
            const string & f_name = ((const asm_label_arg *)args[0])->label;
            for(size_t funcNum = 0; funcNum < functions.size(); funcNum++)
            {
              const asm_function & func = *functions[funcNum];
              if (func.name == f_name) {
                if (func.parameters.size() != (args.size() -1)) {
                  stringstream stream;
                  stream  << "Not enough arguments for function call at:"
                          << endl << f_stmt->position.fileNode->printString()
                            << " Line: " << f_stmt->position.first_line << endl
                          << " - Function '" << f_name << "' requires "
                            << func.parameters.size() << " parameters, while "
                            << (args.size() -1) << " were provided." << endl;
                  throw WrongArgumentException(stream.str());
                }
                break;
              }
            }
          }
        } catch (const WrongArgumentException & e) {
          fprintf(stderr, "ERROR: in instruction!\n%s\n", e.what());
          error = true;
        }
      }
    }
  }
  if (error) {
    throw WrongArgumentException("Errors in instructions definition");
  }
}

void
asm_program::assignFunctionParameters()
{
  for(deque<asm_function *>::const_iterator iter = functions.begin();
      iter != functions.end(); iter++)
  {
    asm_function * func = (*iter);
    for(ListOfStmts::const_iterator stmt_it = func->stmts.begin();
        stmt_it != func->stmts.end(); stmt_it++)
    {
      const asm_statement * stmt = *stmt_it;
      if (stmt->getType() == ASM_FUNCTION_CALL) {
        asm_function_call * f_stmt = (asm_function_call *) stmt;

        const vector<asm_arg *> & args = f_stmt->args;
        const string & f_name = ((const asm_label_arg *)args[0])->label;
        for(size_t funcNum = 0; funcNum < functions.size(); funcNum++)
        {
          const asm_function & func = *functions[funcNum];
          if (func.name == f_name) {
            DebugPrintf(("Found referenced function. Copying params\n"));
            f_stmt->importParameters(func.parameters);
            DebugPrintf(("Copied params\n"));
            break;
          }
        }
      }
    }
  }
}

void
asm_program::ensureTempsUsage(const bool & used) const
{
  bool error = false;
  for(deque<asm_function *>::const_iterator iter = functions.begin();
      iter != functions.end(); iter++)
  {
    error |= (*iter)->ensureTempsUsage(used);
  }
  if (error) {
    throw WrongArgumentException(used
        ? "Explicit registers were found" : "Temporaries were found");
  }
}

void
asm_program::moveMainToTop()
{
  /* Let's put the main function in front of all the others */
  for(deque<asm_function *>::iterator iter = functions.begin();
      iter != functions.end(); iter++)
  {
    if (!(*iter)->name.compare("main")) {
      /* Main found! */
      asm_function * main = (*iter);
      functions.erase(iter);
      functions.insert(functions.begin(), main);
      break;
    }
  }
}

void
asm_program::rebuildOffsets() {
  size_t tempOffset = 0;
  for(size_t index = 0; index < functions.size(); index++) {
    asm_function * func = functions[index];

    func->functionOffset = tempOffset;
    func->rebuildOffsets();

    tempOffset += func->getSize();
  }
  for(size_t index = 0; index < globals.size(); index++) {
    globals[index]->offset = tempOffset;
    tempOffset += globals[index]->getSize();
  }
}

void
asm_program::exposeGlobalLabels()
{
  bool error = false;

  /* And fix their labels */
  DebugPrintf(("-- Adding Functions to Global Labels - Phase --\n"));
  for(size_t index = 0; index < functions.size(); index++) {
    asm_function * func = functions[index];

    asm_label_statement * tempLabel =
        new asm_label_statement(func->position, func->name);
    tempLabel->offset = func->functionOffset;
    func->stmts.push_front(tempLabel);

    try {
      globalSymbols.addLabel(tempLabel);
    } catch (const WrongArgumentException & e) {
      error = true;
      fprintf(stderr, "ERROR:%s at Line %4d\n%s\n--> %s\n",
              tempLabel->position.fileNode->printString().c_str(),
              tempLabel->position.first_line,
              tempLabel->position.fileNode->printStringStackIncludes().c_str(),
              e.what());
    }
  }
  for(size_t index = 0; index < globals.size(); index++) {
    asm_data_statement * stmt = globals[index];

    if (stmt->getType() == ASM_LABEL_STATEMENT) {
      DebugPrintf(("Found label statement: %s!\n",
             ((asm_label_statement *)stmt)->label.c_str()));
      try {
        globalSymbols.addLabel((asm_label_statement *)stmt);
      } catch (const WrongArgumentException & e) {
        error = true;
        fprintf(stderr, "ERROR:%s at Line %4d\n%s\n--> %s\n",
                stmt->position.fileNode->printString().c_str(),
                stmt->position.first_line,
                stmt->position.fileNode->printStringStackIncludes().c_str(),
                e.what());
      }
    }
  }
  DebugPrintf(("-- Terminated: Adding Funcs to Global Labels - Phase --\n\n"));
  if (error) {
    throw WrongArgumentException("Errors in labels definition");
  }
}

void
asm_program::assignValuesToLabels()
{
  bool error = false;

  DebugPrintf(("-- Assign labels - Phase --\n"));
  for(size_t index = 0; index < functions.size(); index++)
  {
    asm_function & func = *functions[index];
    DebugPrintf((" - Processing references of function: %s\n",
                  func.name.c_str()));
    for(list<ArgLabelRecord *>::iterator ref  = func.refs.begin();
        ref != func.refs.end(); ref++)
    {
      asm_label_arg & argument = *((*ref)->arg);
      const string & labelName = argument.label;

      DebugPrintf(("  - Processing label: %s\n", labelName.c_str()));
      asm_label_statement * localLabel = func.localSymbols.getStmt(labelName);
      if (localLabel) {
        DebugPrintf(("    It is local, with position (in the program): %3u\n"
                      "      is const: %5s, is shared: %5s\n",
                      (uint32_t)localLabel->offset,
                      localLabel->is_constant ? "true" : "false",
                      localLabel->is_shared ? "true" : "false"));
        argument.relative = ! localLabel->isShared();
        argument.pointedPosition =
            (int32_t)(localLabel->offset
                      + localLabel->isShared() * func.functionOffset);

      } else {
        DebugPrintf(("    It is not local, trying globally\n"));
        asm_label_statement * globalLabel = globalSymbols.getStmt(labelName);

        if (globalLabel) {
          DebugPrintf(("    It is global, with position: %3u\n",
                      (uint32_t)globalLabel->offset));
          argument.pointedPosition = (int32_t)globalLabel->offset;
          argument.relative = false;
        } else {
          DebugPrintf(("      ERROR!! It is not even global!\n"));
          fprintf(stderr, "ERROR:%s at Line %4d\n%s\n"
                  "--> Reference to not existing Label: '%s'\n",
                  argument.position.fileNode->printString().c_str(),
                  argument.position.first_line,
                  argument.position.fileNode->printStringStackIncludes().c_str(),
                  labelName.c_str());
          error = true;
        }
      }
    }
  }
  DebugPrintf(("-- Terminated: Assign labels - Phase --\n\n"));
  if (error) {
    throw WrongArgumentException("Errors in labels assignment");
  }
}

void
asm_program::assemble(const string & outputName)
{
  Bloat bytecode;
  bytecode.resize(getFunciontsTotalSize() + getGlobalsTotalSize(), 0);
  Bloat::iterator pos = bytecode.begin();
  for (size_t funcIndex = 0; funcIndex < functions.size(); funcIndex++) {
    asm_function & func = *functions[funcIndex];

    for(ListOfStmts::iterator stmt_it = func.stmts.begin();
        stmt_it != func.stmts.end(); stmt_it++)
    {
      asm_statement * stmt = *stmt_it;
      stmt->emitCode(pos);
    }
    for(size_t j = 0; j < func.uniqueLocals.size(); j++) {
      func.uniqueLocals[j]->emitCode(pos);
    }
  }
  for (size_t index = 0; index < globals.size(); index++) {
    globals[index]->emitCode(pos);
  }

  InfoPrintf(("Size of the generated file: %5u bytes in %4u cells\n",
              (uint32_t)bytecode.size()*4, (uint32_t)bytecode.size() ));
#ifdef DEBUG
  DebugPrintf(("-- Dumping generated binary code --\n"));
  for (size_t i = 0; i < bytecode.size(); i++) {
    DebugPrintf(("Mem %03lu: %12d\n", (uint64_t)i, bytecode[i]));
  }

  DebugPrintf(("-- Dumping Partially Disassembled code --\n"));
  Disassembler().disassembleAndPrint(bytecode);
#endif

  BinWriter writer(outputName.c_str());
  writer.saveBinFileContent(bytecode);
}

void
asm_program::emitDebugSymbols(const string & outputName) const
{
  TextWriter writer(outputName);
  writer << "GLOBAL_SYMBOLS" << endl
          << globalSymbols.emitDebugSymbols()
          << "END" << endl << endl;

  for (size_t funcIndex = 0; funcIndex < functions.size(); funcIndex++)
  {
    asm_function & func = *functions[funcIndex];
    writer << "FUNCTION \"" << func.name << "\"" << endl
            << func.localSymbols.emitDebugSymbols()
            << "END" << endl << endl;
  }
}

void
asm_program::emitXMLDebugSymbols(const string & outputName) const
{
  TextWriter writer(outputName);
  writer << "<global_symbols>" << endl
          << globalSymbols.emitXMLDebugSymbols()
          << "</global_symbols>" << endl;

  for (size_t funcIndex = 0; funcIndex < functions.size(); funcIndex++)
  {
    asm_function & func = *functions[funcIndex];
    writer << "<function name=\"" << func.name << "\">" << endl
            << func.localSymbols.emitXMLDebugSymbols()
            << "</function>" << endl;
  }
}


