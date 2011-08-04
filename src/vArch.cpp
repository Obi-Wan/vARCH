/* 
 * File:   vArch.cpp
 * Author: ben
 *
 * Created on 19 agosto 2009, 15.36
 */

#include <stdlib.h>

#include "Chipset.h"
#include "exceptions.h"

#define MAX_MEM 250

/*
 * 
 */
int
main(int argc, char** argv) {
  
  Chipset chipset(MAX_MEM, new int[MAX_MEM]);
  try {
    chipset.startClock();
  } catch (BasicException e) {
    printf("An exception occurred: %s\n", e.what());
  }

  chipset.getCpu().dumpRegistersAndMemory();

  return (EXIT_SUCCESS);
}

