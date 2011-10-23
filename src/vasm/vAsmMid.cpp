/* 
 * File:   vNewAsm.cpp
 * Author: ben
 *
 * Created on 7 agosto 2010, 0.29
 */

#include <cstdlib>
#include "asm-program.h"
#include "IR_Mid_parser.h"
#include "AsmArgs.h"
#include "disassembler/Disassembler.h"

using namespace std;

void
printAssembler(const asm_program * const program);

void
printAbstractTree(const ASTM_Tree * const tree);

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

  if (!openFirstFile(args.getInputName().c_str()))
  {
    fprintf(stderr, "I couldn't open the IRM file to process: %s\n",
            args.getInputName().c_str());
    return (EXIT_FAILURE);
  }
  const bool & usingTemps = args.getRegAutoAlloc();
  try {
    ASTM_Tree * ast_tree = new ASTM_Tree();
    int res = yyparse(ast_tree);
    if (res) {
      fprintf(stderr, "An error may have occurred, code: %3d\n", res);
      throw BasicException("Error parsing\n");
    }
#ifdef DEBUG
    printAbstractTree(ast_tree);
#endif
//    asm_program * program = new asm_program();
//    ast_tree->emitAsm(*program);
//    delete ast_tree;
//
//    program->moveMainToTop();
//    program->checkInstructions(usingTemps);
//#ifdef DEBUG
//    printAssembler(program);
//#endif
//
//    if (usingTemps) {
//      program->assignFunctionParameters();
//      program->doRegisterAllocation(args);
//    } else {
//      program->ensureTempsUsage(usingTemps);
//    }
//
//    program->rebuildOffsets();
//    program->exposeGlobalLabels();
//    program->assignValuesToLabels();
//    program->assemble( args.getOutputName() );
//
//#ifdef DEBUG
//    Disassembler().disassembleProgram(*program);
//#endif
//
//    if (!args.getDebugSymbolsName().empty()) {
//      const string & symName = args.getDebugSymbolsName();
//      const size_t & symNameSize = symName.size();
//      if (symNameSize > 4 && !symName.substr(symNameSize-4,4).compare(".xml"))
//      {
//        program->emitXMLDebugSymbols(args.getDebugSymbolsName());
//      } else {
//        program->emitDebugSymbols(args.getDebugSymbolsName());
//      }
//    }
//    cleanParser();

  } catch (const BasicException & e) {
    fprintf(stderr, "Error: %s\n", e.what());
    return (EXIT_FAILURE);
  }
}

void
printAbstractTree(const ASTM_Tree * const tree)
{
#ifdef DEBUG
  DebugPrintf(("-- Dumping Schematic Parsed - Mid IR --\n"));

  DebugPrintf((" - Prototypes: -\n"));
  for(size_t funcNum = 0; funcNum < tree->functionProtos.size(); funcNum++) {
    const ASTM_FunctionProto & func = *tree->functionProtos[funcNum];
    DebugPrintf(("Line: %03d Function: %s, Ret Type: %d\n", func.pos.first_line,
                  func.name.c_str(), func.retType));
    DebugPrintf(("   Parameters:\n"));
    for(vector<ASTM_Param *>::const_iterator stmt_it = func.params.begin();
        stmt_it != func.params.end(); stmt_it++)
    {
      const ASTM_Param * param = *stmt_it;
      DebugPrintf((" Line: %03d %s, type %d\n", param->pos.first_line,
                    param->toString().c_str(), param->type));
    }
  }
  DebugPrintf((" - End of Prototypes: -\n\n"));

  DebugPrintf((" - Globals: -\n"));
  for(size_t num = 0; num < tree->globals.size(); num++) {
    DebugPrintf(("Line: %03d Global: Name: %s, info: %s\n",
                  tree->globals[num]->pos.first_line,
                  tree->globals[num]->assign->assignedId.c_str(),
                  tree->globals[num]->toString().c_str()));
  }
  DebugPrintf((" - End of Globals: -\n\n"));

  DebugPrintf((" - Function Definitions: -\n"));
  for(size_t funcNum = 0; funcNum < tree->functionDefs.size(); funcNum++) {
    const ASTM_FunctionDef & func = *tree->functionDefs[funcNum];
    DebugPrintf(("Line: %03d Function: %s, Ret Type: %d\n", func.pos.first_line,
                  func.name.c_str(), func.retType));
    DebugPrintf(("   Parameters:\n"));
    for(vector<ASTM_Param *>::const_iterator stmt_it = func.params.begin();
        stmt_it != func.params.end(); stmt_it++)
    {
      const ASTM_Param * param = *stmt_it;
      DebugPrintf((" Line: %03d %s, type %d\n", param->pos.first_line,
                    param->toString().c_str(), param->type));
    }
    DebugPrintf(("   Statements:\n"));
    for(vector<ASTM_Stmt *>::const_iterator stmt_it = func.stmts.begin();
        stmt_it != func.stmts.end(); stmt_it++)
    {
      const ASTM_Stmt * stmt = *stmt_it;
      DebugPrintf((" Line: %03d %s\n", stmt->pos.first_line,
                    stmt->toString().c_str()));
    }
  }
  DebugPrintf((" - End of Function Definitions: -\n\n"));

  DebugPrintf(("-- Terminated Dumping Parsed Code --\n\n"));
#endif
}

void
printAssembler(const asm_program * const program) {
#ifdef DEBUG
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
#endif
}

