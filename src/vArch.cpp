/* 
 * File:   vArch.cpp
 * Author: ben
 *
 * Created on 19 agosto 2009, 15.36
 */

#include <stdlib.h>

#include "Chipset.h"

#define MAX_MEM 100

/*
 * 
 */
int
main(int argc, char** argv) {
  
  Chipset chipset(MAX_MEM, new int[MAX_MEM]);
  chipset.startClock();

  chipset.getCpu().dumpRegistersAndMemory();

  return (EXIT_SUCCESS);
}

