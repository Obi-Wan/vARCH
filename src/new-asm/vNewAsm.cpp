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

using namespace std;

YYLTYPE yylloc = { 1, 1, 1, 1 };

void
printAbstractTree(const asm_program * const program);

/*
 * 
 */
int
main(int argc, char** argv) {

  AsmArgs args(argc, argv);
  try {
    args.parse();
  } catch (const WrongArgumentException & e) {
    if (e.getMessage().empty()) {
      args.printHelp();
      return EXIT_SUCCESS;
    } else {
      printf("%s\n", e.what());
      args.printHelp();
      return EXIT_FAILURE;
    }
  }
  setIncludeDirs(&args.getIncludeDirs());

  if (openIncludeFile(args.getInputName().c_str(), &yylloc))
  {
    try {
      asm_program * program;
      int res = yyparse(program);
      if (res) {
        printf("An error may have occurred, code: %3d\n", res);
        throw BasicException("Error parsing\n");
      }
#ifdef DEBUG
      printAbstractTree(program);
#endif
      program->assignValuesToLabels();
      program->assemble( args.getOutputName() );
    } catch (BasicException e) {
      printf("Error: %s\n", e.what());
      return (EXIT_FAILURE);
    }
  } else {
    printf("I couldn't open the ASM file to process: %s\n",
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

