/* 
 * File:   asm_program.cpp
 * Author: ben
 * 
 * Created on 5 agosto 2010, 21.05
 */

#include "asm-program.h"

#include "IncludesTree.h"
#include "../FileHandler.h"
#include "backend/AssemFlowGraph.h"
#include "backend/RegAllocator.h"
#include "backend/Frame.h"
#include "backend/Optimizer.h"

#include "ErrorReporter.h"

#include <sstream>
using namespace std;

asm_program::~asm_program()
{
  for(asm_function * func : functions) { delete func; }
  for(asm_data_statement * stmt : globals) { delete stmt; }
}

void
asm_program::checkInstructions(const bool & usingTemps) const
{
  bool error = false;
  for(asm_function * func : functions)
  {
    for(const asm_statement * stmt : func->stmts)
    {
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
  for(asm_function * func : functions)
  {
    for(asm_statement * stmt : func->stmts)
    {
      if (stmt->getType() == ASM_FUNCTION_CALL) {
        asm_function_call * f_stmt = (asm_function_call *) stmt;

        const vector<asm_arg *> & args = f_stmt->args;
        const string & f_name = ((const asm_label_arg *)args[0])->label;

        for(const asm_function * o_func : functions)
        {
          if (o_func->name == f_name) {
            DebugPrintf(("Found referenced function. Copying params\n"));
            f_stmt->importParameters(o_func->parameters);
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
  for(asm_function * func : functions)
  {
    error |= func->ensureTempsUsage(used);
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
asm_program::doRegisterAllocation(const AsmArgs & args)
{
  bool error = false;

  for(size_t numFunc = 0; numFunc < functions.size(); numFunc++)
  {
    asm_function & func = *(functions[numFunc]);
    DebugPrintf(("\n\n"));
    DebugPrintf(("--> PROCESSING FUNCTION: \"%s\"\n\n", func.name.c_str()));
    try {

      TempsMap tempsMap;

      Frame frame(tempsMap);
      frame.init(func);
      frame.generateMovesForFunctionCalls(func);
      frame.allocateLocalVariables(func);
      frame.deallocateLocalVariables(func);

      AssemFlowGraph flowGraph(tempsMap);
      flowGraph.populateGraph(func);

#ifdef DEBUG
      DebugPrintf((" --> Printing Flow Graph!! <--\n"));
      flowGraph.printFlowGraph();
      DebugPrintf((" --> Printed Flow Graph!! <--\n\n"));
#endif

      LiveMap<asm_statement *> liveMap;
      flowGraph.populateLiveMap(liveMap);

#ifdef DEBUG
      DebugPrintf((" --> Printing Live Map!! <--\n"));
      liveMap.printLiveMap();
      DebugPrintf((" --> Printed Live Map!! <--\n\n"));
#endif

      flowGraph.checkTempsUsedUndefined(func, liveMap);

      InterferenceGraph interfGraph;
      interfGraph.populateGraph( flowGraph, liveMap, tempsMap);

#ifdef DEBUG
      DebugPrintf((" --> Printing Interference Graph!! <--\n"));
      interfGraph.printInterferenceGraph();
      DebugPrintf((" --> Printed Interference Graph!! <--\n\n"));
#endif

      RegAllocator regAlloc(tempsMap);
      DebugPrintf((" --> Printing Allocator Stack!! <--\n"));
      bool success = args.getRegCoalesce()
          ? regAlloc.coalesceAllocateRegs(interfGraph)
          : regAlloc.simpleAllocateRegs(interfGraph);
      DebugPrintf((" --> Printed Allocator Stack!! <--\n\n"));

      if (success) {
        if (args.getRegCoalesce()) {
          regAlloc.getAliases().print();
          regAlloc.getReverseAliases().print();

          flowGraph.applySelectedRegisters(regAlloc.getAssignedRegs(),
              regAlloc.getAliases());
        } else {
          flowGraph.applySelectedRegisters(regAlloc.getAssignedRegs());
        }
      } else {
        throw WrongArgumentException(
            "Couldn't find a registers allocation solution for function "
            + func.name);
      }

      Optimizer optimizer;
      switch (args.getOptimizationLevel()) {
        case 3:
        case 2:
        case 1: {
          optimizer.removeUselessMoves(func);
          optimizer.removeUselessArithmetics(func);
        }
        default: break;
      }
    } catch (const BasicException & e) {
      fprintf(stderr, "Error in function \"%s\": %s\n", func.name.c_str(),
              e.what());
      error = true;
    }
  }
  if (error) {
    throw BasicException("Errors in register allocation!");
  }
}

void
asm_program::rebuildOffsets()
{
  size_t tempOffset = 0;
  for(asm_function * const func : functions)
  {
    func->functionOffset = tempOffset;
    func->rebuildOffsets();

    tempOffset += func->getSize();
  }
  for(asm_data_statement * const stmt : globals) {
    stmt->offset = tempOffset;
    tempOffset += stmt->getSize();
  }
}

void
asm_program::exposeGlobalLabels()
{
  ErrorReporter errOut;

  /* And fix their labels */
  DebugPrintf(("-- Adding Functions to Global Labels - Phase --\n"));

  for(asm_function * const func : functions)
  {
    asm_label_statement * const tempLabel =
        new asm_label_statement(func->position, func->name, true);
    tempLabel->offset = func->functionOffset;
    func->stmts.push_front(tempLabel);

    try {
      globalSymbols.addLabel(tempLabel);
    } catch (const WrongArgumentException & e) {
      errOut.addErrorMsg(tempLabel->position, e);
    }
  }
  for(asm_data_statement * const stmt : globals)
  {
    try {
      if (stmt->getType() == ASM_LABEL_STATEMENT) {
        asm_label_statement * l_stmt = (asm_label_statement *) stmt;
        DebugPrintf(("Found label statement: %s!\n", l_stmt->label.c_str() ));

        globalSymbols.addLabel(l_stmt);
      }
    } catch (const WrongArgumentException & e) {
      errOut.addErrorMsg(stmt->position, e);
    }
  }
  DebugPrintf(("-- Terminated: Adding Funcs to Global Labels - Phase --\n\n"));
  if (errOut.hasErrors()) {
    errOut.throwError( WrongArgumentException("Errors in labels definition") );
  }
}

void
asm_program::assignValuesToLabels()
{
  bool error = false;

  DebugPrintf(("-- Assign labels - Phase --\n"));

  for(asm_function * func : functions)
  {
    DebugPrintf((" - Processing references of function: %s\n",
                  func->name.c_str()));
    for(ArgLabelRecord * ref : func->refs)
    {
      asm_label_arg & argument = *(ref->arg);
      const string & labelName = argument.label;

      DebugPrintf(("  - Processing label: %s\n", labelName.c_str()));
      asm_label_statement * localLabel = func->localSymbols.getStmt(labelName);
      if (localLabel) {
        DebugPrintf(("    It is local, with position (in the program): %3u\n"
                      "      is const: %5s, is shared: %5s\n",
                      (uint32_t)localLabel->offset,
                      localLabel->is_constant ? "true" : "false",
                      localLabel->is_shared ? "true" : "false"));
        if (localLabel->isShared()) {
          /* TODO: add displaced referencing to Program Counter */
          argument.content.val =
                        (int32_t)(localLabel->offset + func->functionOffset);
        } else {
          argument.type = DISPLACED;
          argument.content.regNum = STACK_POINTER;
          argument.displacement = (int32_t)(func->getStackedDataSize() - localLabel->offset);
        }

      } else {
        DebugPrintf(("    It is not local, trying globally\n"));
        asm_label_statement * globalLabel = globalSymbols.getStmt(labelName);

        if (globalLabel) {
          DebugPrintf(("    It is global, with position: %3u\n",
                      (uint32_t)globalLabel->offset));
          argument.content.val = (int32_t)globalLabel->offset;
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

  for(asm_function * func : functions)
  {
    for(asm_statement * stmt : func->stmts) { stmt->emitCode(pos); }
    for(asm_data_statement * stmt : func->uniqueLocals) { stmt->emitCode(pos); }
  }
  for (asm_data_statement * stmt : globals) { stmt->emitCode(pos); }

  InfoPrintf(("Size of the generated file: %5u bytes\n",
              (uint32_t)bytecode.size() ));

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

  for(asm_function * func : functions)
  {
    writer << "FUNCTION \"" << func->name << "\"" << endl
            << func->localSymbols.emitDebugSymbols()
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

  for(asm_function * func : functions)
  {
    writer << "<function name=\"" << func->name << "\">" << endl
            << func->localSymbols.emitXMLDebugSymbols()
            << "</function>" << endl;
  }
}


