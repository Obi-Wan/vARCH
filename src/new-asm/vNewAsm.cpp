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

extern FILE *yyin;

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

  yyin = fopen( args.getInputName().c_str(), "r" );
  if (yyin != NULL) {
    try {
      asm_program * program;
      int res = yyparse(program);
      if (!res) {
        for(int i = 0; i < program->functions.size(); i++) {
          DebugPrintf(("Line: %03d Function: %s\n",
                        program->functions[i]->position.first_line,
                        program->functions[i]->name.c_str()));
          for(int j = 0; j < program->functions[i]->stmts.size(); j++) {
            DebugPrintf((" Line: %03d %s\n",
                          program->functions[i]->stmts[j]->position.first_line,
                          program->functions[i]->stmts[j]->toString().c_str()));
          }
          for(int j = 0; j < program->functions[i]->locals.size(); j++) {
            DebugPrintf((" Line: %03d Local: %s\n",
                          program->functions[i]->locals[j]->position.first_line,
                          program->functions[i]->locals[j]->toString().c_str()));
          }
        }
        for(int i = 0; i < program->globals.size(); i++) {
          DebugPrintf(("Line: %03d Global: %s\n",
                        program->globals[i]->position.first_line,
                        program->globals[i]->toString().c_str()));
        }
      } else {
        printf("An error may have occurred, code: %3d\n", res);
      }
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

