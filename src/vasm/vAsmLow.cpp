/* 
 * File:   vNewAsm.cpp
 * Author: ben
 *
 * Created on 7 agosto 2010, 0.29
 */

#include <cstdlib>
#include "asm-program.h"
#include "IR_Low_parser.h"
#include "AST/AST_Low/AST_Low_Tree.h"
#include "AsmArgs.h"
#include "disassembler/Disassembler.h"

using namespace std;

void
printAssembler(const asm_program & program);

void
printDefines(const AsmPreprocessor & defs);

/*
 * 
 */
int
main(int argc, char** argv)
{
  AsmArgs args(argc, argv);
  AsmPreprocessor defines;
  defines.addDefine(string("VASM_VERSION"), VERSION);

  try {
    args.parse();
  } catch (const WrongArgumentException & e) {
    if (e.getMessage().empty()) {
      args.printHelp();
      return EXIT_SUCCESS;
    } else {
      fprintf(stderr, "%s\n", e.what());
      args.printHelp();
      return EXIT_FAILURE;
    }
  }
  setIncludeDirs(&args.getIncludeDirs());

  if (!openFirstFile(args.getInputName().c_str()))
  {
    fprintf(stderr, "I couldn't open the ASM file to process: %s\n",
            args.getInputName().c_str());
    return (EXIT_FAILURE);
  }
  const bool & usingTemps = args.getRegAutoAlloc();
  try {
    ASTL_Tree ast_tree;

    int res = yyparse(ast_tree, defines);
    if (res) {
      fprintf(stderr, "An error may have occurred, code: %3d\n", res);
      throw BasicException("Error parsing\n");
    }
    if (args.getOnlyValidate()) {
      ast_tree.printTree();
    } else {
      asm_program program;
      ast_tree.emitAsm(program);

      program.moveMainToTop();
      program.checkInstructions(usingTemps);
#ifdef DEBUG
      printAssembler(program);
#endif

      if (usingTemps) {
        program.assignFunctionParameters();
        program.doRegisterAllocation(args);
      } else {
        program.ensureTempsUsage(usingTemps);
      }

      program.rebuildOffsets();
      program.exposeGlobalLabels();
      program.assignValuesToLabels(args);
      program.assemble( args.getOutputName() );

      if (args.getDisassembleResult()) {
        Disassembler().disassembleProgram(program);
      }

      if (!args.getDebugSymbolsName().empty()) {
        const string & symName = args.getDebugSymbolsName();
        const size_t & symNameSize = symName.size();
        if (symNameSize > 4 && !symName.substr(symNameSize-4,4).compare(".xml"))
        {
          program.emitXMLDebugSymbols(args.getDebugSymbolsName());
        } else {
          program.emitDebugSymbols(args.getDebugSymbolsName());
        }
      }
    }
    cleanParser();
    printDefines(defines);

  } catch (const BasicException & e) {
    fprintf(stderr, "Error: %s\n", e.what());
    return (EXIT_FAILURE);
  }
}

void
printAssembler(const asm_program & program)
{
#ifdef DEBUG
  DebugPrintf(("-- Dumping Schematic Parsed Code --\n"));
  for(const asm_function * func : program.functions)
  {
    DebugPrintf(("Line: %03d Function: %s\n", func->position.first_line,
                  func->name.c_str()));
    for(const asm_statement * stmt : func->stmts)
    {
      DebugPrintf((" Line: %03d %s\n", stmt->position.first_line,
                    stmt->toString().c_str()));
    }
    for(asm_data_statement * stmt : func->uniqueLocals)
    {
      DebugPrintf((" Line: %03d Local: %s\n",
          stmt->position.first_line, stmt->toString().c_str()));
    }
    for(asm_data_statement * stmt : func->stackLocals)
    {
      DebugPrintf((" Line: %03d Local: %s\n",
          stmt->position.first_line, stmt->toString().c_str()));
    }
  }
  for(asm_data_statement * stmt : program.globals)
  {
    DebugPrintf(("Line: %03d Global: %s\n",
        stmt->position.first_line, stmt->toString().c_str()));
  }
  DebugPrintf(("-- Terminated Dumping Parsed Code --\n\n"));
#endif
}

void
printDefines(const AsmPreprocessor & defs)
{
  const DefineType & d = defs.getDefines();
  DebugPrintf(("Num of Defines %u\n", d.size()));
  for(const auto entry : d) {
    string argsDescr;
    for(const string param : entry.second.parameters) {
      argsDescr += " ";
      argsDescr += param;
    }
    DebugPrintf(("- Name: \"%s\"\n", entry.first.c_str() ));
    DebugPrintf(("  Params: \"%s\"\n", argsDescr.c_str() ));
    DebugPrintf(("  Definition: \"%s\"\n", entry.second.content.c_str() ));
  }
}

