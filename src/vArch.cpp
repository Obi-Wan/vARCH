/* 
 * File:   vArch.cpp
 * Author: ben
 *
 * Created on 19 agosto 2009, 15.36
 */

#include <stdlib.h>

#include "Chipset.h"
#include "exceptions.h"

#define NUM_MEM_BYTES 350
#define MAX_MEM_ADDR  (NUM_MEM_BYTES * 4)

/*
 * 
 */
int
main(int argc, char** argv) {
  
  Chipset chipset(MAX_MEM_ADDR, new DoubleWord[NUM_MEM_BYTES]);
  try {
    chipset.startClock();
  } catch (const BasicException & e) {
    printf("An exception occurred: %s\n", e.what());
  }

  chipset.getCpu().dumpRegistersAndMemory();

  return (EXIT_SUCCESS);
}

