/* 
 * File:   vNewAsm.cpp
 * Author: ben
 *
 * Created on 7 agosto 2010, 0.29
 */

#include <cstdlib>
#include "asm-program.h"
#include "asm-parser.h"
#include "AsmArgs.h"
#include "backend/AssemFlowGraph.h"
#include "backend/RegAllocator.h"
#include "backend/StaticLinker.h"
#include "backend/Optimizer.h"

using namespace std;

void
printAbstractTree(const asm_program * const program);

/*
 * 
 */
int
main(int argc, char** argv)
{
  AsmArgs args(argc, argv);
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

  if (openFirstFile(args.getInputName().c_str()))
  {
    const bool & usingTemps = args.getRegAutoAlloc();
    try {
      asm_program * program = new asm_program();
      int res = yyparse(program);
      if (res) {
        fprintf(stderr, "An error may have occurred, code: %3d\n", res);
        throw BasicException("Error parsing\n");
      }
      program->moveMainToTop();
      program->checkInstructions(usingTemps);
#ifdef DEBUG
      printAbstractTree(program);
#endif

      if (usingTemps) {
        program->assignFunctionParameters();

        for(size_t numFunc = 0; numFunc < program->functions.size(); numFunc++)
        {
          asm_function & func = *(program->functions[numFunc]);
          TempsMap tempsMap;

          StaticLinker linker(tempsMap);
          linker.init(func);
          linker.generateMovesForFunctionCalls(func);
          linker.allocateLocalVariables(func);
          linker.deallocateLocalVariables(func);

          AssemFlowGraph flowGraph(tempsMap);
          flowGraph.populateGraph(func);

          DebugPrintf((" --> Printing Flow Graph!! <--\n"));
          flowGraph.printFlowGraph();
          DebugPrintf((" --> Printed Flow Graph!! <--\n\n"));

          LiveMap<asm_statement *> liveMap;
          flowGraph.populateLiveMap(liveMap);

          DebugPrintf((" --> Printing Live Map!! <--\n"));
          liveMap.printLiveMap();
          DebugPrintf((" --> Printed Live Map!! <--\n\n"));

          InterferenceGraph interfGraph;
          interfGraph.populateGraph( flowGraph, liveMap, tempsMap);

          DebugPrintf((" --> Printing Interference Graph!! <--\n"));
          interfGraph.printInterferenceGraph();
          DebugPrintf((" --> Printed Interference Graph!! <--\n\n"));

          RegAllocator regAlloc(tempsMap);
          DebugPrintf((" --> Printing Allocator Stack!! <--\n"));
          bool success = regAlloc.simpleAllocateRegs(interfGraph);
          DebugPrintf((" --> Printed Allocator Stack!! <--\n\n"));

          if (success) {
            flowGraph.applySelectedRegisters(regAlloc.getAssignedRegs());
          } else {
            throw WrongArgumentException(
                "Couldn't find a solution for registers allocation");
          }

          Optimizer optimizer;
          switch (args.getOptimizationLevel()) {
            case 3:
            case 2:
            case 1: {
              optimizer.removeUselessMoves(func);
              optimizer.removeUselessArithemtics(func);
            }
            default: break;
          }
        }
        program->rebuildFunctionsOffsets();
      } else {
        program->ensureTempsUsage(usingTemps);
      }

      program->addFunctionLabelsToGlobals();
      program->assignValuesToLabels();
      program->assemble( args.getOutputName() );

      if (!args.getDebugSymbolsName().empty()) {
        const string & symName = args.getDebugSymbolsName();
        const size_t & symNameSize = symName.size();
        if (symNameSize > 4 && !symName.substr(symNameSize-4,4).compare(".xml"))
        {
          program->emitXMLDebugSymbols(args.getDebugSymbolsName());
        } else {
          program->emitDebugSymbols(args.getDebugSymbolsName());
        }
      }
      cleanParser();

    } catch (BasicException e) {
      fprintf(stderr, "Error: %s\n", e.what());
      return (EXIT_FAILURE);
    }
  } else {
    fprintf(stderr, "I couldn't open the ASM file to process: %s\n",
            args.getInputName().c_str());
    return (EXIT_FAILURE);
  }
}

void
printAbstractTree(const asm_program * const program) {
  DebugPrintf(("-- Dumping Schematic Parsed Code --\n"));
  for(size_t funcNum = 0; funcNum < program->functions.size(); funcNum++) {
    const asm_function & func = *program->functions[funcNum];
    DebugPrintf(("Line: %03d Function: %s\n", func.position.first_line,
                  func.name.c_str()));
    for(ListOfStmts::const_iterator stmt_it = func.stmts.begin();
        stmt_it != func.stmts.end(); stmt_it++)
    {
      const asm_statement * stmt = *stmt_it;
      DebugPrintf((" Line: %03d %s\n", stmt->position.first_line,
                    stmt->toString().c_str()));
    }
    for(size_t localNum = 0; localNum < func.uniqueLocals.size(); localNum++) {
      DebugPrintf((" Line: %03d Local: %s\n",
                    func.uniqueLocals[localNum]->position.first_line,
                    func.uniqueLocals[localNum]->toString().c_str()));
    }
    for(size_t localNum = 0; localNum < func.stackLocals.size(); localNum++) {
      DebugPrintf((" Line: %03d Local: %s\n",
                    func.stackLocals[localNum]->position.first_line,
                    func.stackLocals[localNum]->toString().c_str()));
    }
  }
  for(size_t num = 0; num < program->globals.size(); num++) {
    DebugPrintf(("Line: %03d Global: %s\n",
                  program->globals[num]->position.first_line,
                  program->globals[num]->toString().c_str()));
  }
  DebugPrintf(("-- Terminated Dumping Parsed Code --\n\n"));
}

