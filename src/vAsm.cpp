/* 
 * File:   vAsm.cpp
 * Author: ben
 *
 * Created on 27 agosto 2009, 14.25
 */

#include <stdlib.h>
#include <stdio.h>

#include "AsmParser.h"

/*
 * 
 */
int
main(int argc, char** argv) {

  if (argc < 2) {
    printf("You didn't enter the ASM file to process\n");
    return (EXIT_FAILURE);
  }

  AsmParser parser(argv[1]);
  try {
    parser.parse();
    parser.assemble();
  } catch (BasicException e) {
    printf("Error: %s\n", e.what());
  }

  return (EXIT_SUCCESS);
}

