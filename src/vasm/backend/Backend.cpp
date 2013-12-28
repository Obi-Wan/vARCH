/*
 * Backend.cpp
 *
 *  Created on: 10/feb/2013
 *      Author: ben
 */

#include "Backend.h"

#include "AsmChecker.h"
#include "Frame.h"
#include "RegAllocator.h"
#include "Optimizer.h"
#include "Linker.h"
#include "ObjHandler.h"
#include "../disassembler/Disassembler.h"

void
Backend::sourceAST(const ASTL_Tree & tree)
{
  tree.emitAsm(program);
}

INLINE void
Backend::assignFunctionParameters()
{
  for(asm_function * func : program.functions)
  {
    for(asm_statement * stmt : func->stmts)
    {
      if (stmt->getType() == ASM_FUNCTION_CALL) {
        asm_function_call * f_stmt = (asm_function_call *) stmt;

        const vector<asm_arg *> & args = f_stmt->args;
        const string & f_name = ((const asm_label_arg *)args[0])->label;

        for(const asm_function * o_func : program.functions)
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

INLINE void
Backend::doRegisterAllocation()
{
  bool error = false;

  for(asm_function * pFunc : program.functions)
  {
    asm_function & func = * pFunc;
    DebugPrintf(("\n\n"));
    DebugPrintf(("--> PROCESSING FUNCTION: \"%s\"\n\n", func.name.c_str()));
    try {

      TempsMap tempsMap;

      Frame frame(tempsMap);
      frame.init(func);
      frame.generateMovesForFunctionCalls(func);
      frame.allocateLocalVariables(func);
      frame.deallocateLocalVariables(func);
      if (!args.getOmitFramePointer()) {
        frame.updateFramePointer(func);
      }

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
Backend::loadObj(const string & filename)
{
  asm_program prog;
  ObjLoader reader(filename);
  reader.readObj(prog);

  // Let's merge the two into "this->program"
  program.functions.insert(program.functions.begin(), prog.functions.begin(),
      prog.functions.end());
  program.shared_vars.insert(program.shared_vars.begin(), prog.shared_vars.begin(),
      prog.shared_vars.end());
  program.constants.insert(program.constants.begin(), prog.constants.begin(),
      prog.constants.end());

  // Let's clear temporary object, so that it doesn't dispose the merged objects
  prog.functions.clear();
  prog.shared_vars.clear();
  prog.constants.clear();
}

void
Backend::saveObj(const string & filename)
{
  ObjWriter writer(filename);
  writer.writeObj(this->program);
}

void
Backend::emit()
{
  const bool usingTemps = args.getRegAutoAlloc();

  AsmChecker::checkInstructions(program, usingTemps);

  if (usingTemps)
  {
    this->assignFunctionParameters();
    this->doRegisterAllocation();
  }
  else
  {
    AsmChecker::ensureTempsUsage(program, usingTemps);
  }

  Linker linker(program, args);
  linker.prelink();

  for(const string & filename : args.getObjInputNames())
  {
    DebugPrintf(("LOADING: %s\n", filename.c_str()));
    this->loadObj(filename);
  }

  const string & output_name = args.getOutputName();

  if (args.getOnlyCompile())
  {
    this->saveObj( output_name );
  }
  else
  {
    linker.link();

    program.assemble( output_name );

    if (args.getDisassembleResult())
    {
      Disassembler().disassembleProgram(program);
    }
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
