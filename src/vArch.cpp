/* 
 * File:   vArch.cpp
 * Author: ben
 *
 * Created on 19 agosto 2009, 15.36
 */

#include <stdlib.h>

#include "Chipset.h"

/*
 * 
 */
int
main(int argc, char** argv) {
  
  Chipset chipset(1000);
  chipset.startClock();

  return (EXIT_SUCCESS);
}

