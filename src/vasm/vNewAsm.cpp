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
    try {
      asm_program * program = new asm_program();
      int res = yyparse(program);
      if (res) {
        fprintf(stderr, "An error may have occurred, code: %3d\n", res);
        throw BasicException("Error parsing\n");
      }
      program->checkInstructions();
      program->addFunctionLabelsToGlobals();
#ifdef DEBUG
      printAbstractTree(program);
#endif
      if (args.getRegAutoAlloc()) {
        AssemFlowGraph flowGraph;
        flowGraph.populateGraph(*(program->functions[0]));
        DebugPrintf((" --> Printing Flow Graph!! <--\n"));
        flowGraph.printFlowGraph();
        DebugPrintf((" --> Printed Flow Graph!! <--\n\n"));

        LiveMap<asm_statement *> liveMap;
        flowGraph.populateLiveMap(liveMap);
        DebugPrintf((" --> Printing Live Map!! <--\n"));
        liveMap.printLiveMap();
        DebugPrintf((" --> Printed Live Map!! <--\n\n"));

        InteferenceGraph interfGraph;
        interfGraph.populateGraph<asm_statement *>( flowGraph, liveMap,
                                                    flowGraph.getTempsMap());
        DebugPrintf((" --> Printing Interference Graph!! <--\n"));
        interfGraph.printInterferenceGraph();
        DebugPrintf((" --> Printed Interference Graph!! <--\n\n"));

        RegAllocator regAlloc(flowGraph.getTempsMap());
        DebugPrintf((" --> Printing Allocator Stack!! <--\n"));
        regAlloc.simpleAllocateRegs(interfGraph);
        DebugPrintf((" --> Printed Allocator Stack!! <--\n\n"));

        flowGraph.applySelectedRegisters(regAlloc.getAssignedRegs());
      }

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
    for(size_t stmtNum = 0; stmtNum < func.stmts.size(); stmtNum++) {
      DebugPrintf((" Line: %03d %s\n",
                    func.stmts[stmtNum]->position.first_line,
                    func.stmts[stmtNum]->toString().c_str()));
    }
    for(size_t localNum = 0; localNum < func.locals.size(); localNum++) {
      DebugPrintf((" Line: %03d Local: %s\n",
                    func.locals[localNum]->position.first_line,
                    func.locals[localNum]->toString().c_str()));
    }
  }
  for(size_t num = 0; num < program->globals.size(); num++) {
    DebugPrintf(("Line: %03d Global: %s\n",
                  program->globals[num]->position.first_line,
                  program->globals[num]->toString().c_str()));
  }
  DebugPrintf(("-- Terminated Dumping Parsed Code --\n\n"));
}

