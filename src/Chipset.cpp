/* 
 * File:   Chipset.cpp
 * Author: ben
 * 
 * Created on 20 agosto 2009, 14.51
 */

#include <unistd.h>
#include <stdio.h>

#include "Chipset.h"
#include "../include/StdIstructions.h"

Chipset::Chipset(const int _maxMem) : cpu(*(new Cpu(*(new Mmu(_maxMem,mainMem))))) {
  mainMem = new int[_maxMem];
  for (int i = 0; bios[i] != 0 || bios[i+1] != 0; i++) {
    mainMem[i] = bios[i];
  }
}

//Chipset::Chipset(const Chipset& orig) { }

Chipset::~Chipset() { }

const int
Chipset::bios[] = { 0, 0, 0 };

void
Chipset::startClock() {
  printf("Starting execution\n");

  int result = 0;
  do {
    result = cpu.coreStep();
    sleep(10);
  } while (result != HALT);

  printf("Exiting\n");
}


void
Chipset::addComponent(Component &) { }

//void
//Chipset::addCpu(Cpu & _cpu) {
//  cpu = _cpu;
//}
