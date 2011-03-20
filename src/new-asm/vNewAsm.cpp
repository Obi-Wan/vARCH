/* 
 * File:   vNewAsm.cpp
 * Author: ben
 *
 * Created on 7 agosto 2010, 0.29
 */

#include <cstdlib>
#include "asm-program.h"
#include "asm-parser.h"

using namespace std;

extern FILE *yyin;

/*
 * 
 */
int
main(int argc, char** argv) {

  if (argc < 2) {
    printf("You didn't enter the ASM file to process\n");
    return (EXIT_FAILURE);
  }
  ++argv, --argc;
  yyin = fopen( argv[0], "r" );
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
      program->assemble( argv[0] );
    } catch (BasicException e) {
      printf("Error: %s\n", e.what());
      return (EXIT_FAILURE);
    }
  } else {
    printf("I couldn't open the ASM file to process\n");
    return (EXIT_FAILURE);
  }
}

