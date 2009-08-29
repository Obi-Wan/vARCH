/* 
 * File:   Chipset.cpp
 * Author: ben
 * 
 * Created on 20 agosto 2009, 14.51
 */

#include <unistd.h>
#include <stdio.h>

#include "Chipset.h"
#include "../include/std_istructions.h"
#include "../include/asm_helpers.h"

Chipset::Chipset(const int& _maxMem, int * _mainMem)
    : cpu(* (new Cpu(*this,*(new Mmu(_maxMem,_mainMem))))), mainMem(_mainMem)
      , maxMem(_maxMem) {

  initMem();
  cpu.dumpRegistersAndMemory();
}

//Chipset::Chipset(const Chipset& orig) { }

Chipset::~Chipset() { }

//const int
//Chipset::bios[] = {
//  MOV + ARG_1(COST) + ARG_2(REG), 10, REG_DATA_1,
//  MOV + ARG_1(COST) + ARG_2(REG), 20, REG_DATA_2,
//  ADD + ARG_1(REG) + ARG_2(REG), REG_DATA_1, REG_DATA_2,
//  MOV + ARG_1(REG) + ARG_2(ADDR), REG_DATA_2, 16,
//  HALT, 0, 0 };

const int
Chipset::bios[] = {
  MOV + ARG_1(COST) + ARG_2(REG),                   7,          REG_DATA_1,
  MOV + ARG_1(COST) + ARG_2(REG),                   1,          REG_DATA_2,
  IFEQJ + ARG_1(COST) + ARG_2(REG) + ARG_3(COST),   17,         REG_DATA_1, 0,
  MULT + ARG_1(REG_POST_DECR) + ARG_2(REG),         REG_DATA_1, REG_DATA_2,
  IFNEQJ + ARG_1(COST) + ARG_2(REG) + ARG_3(COST),  10,         REG_DATA_1, 0,
  MOV + ARG_1(REG) + ARG_2(ADDR),                   REG_DATA_2, 23,
  HALT, 0, 0 };

Bloat
Chipset::loadBiosFromFile(const char * file) {
  BinLoader handler(file);
  return handler.getBinFileContent();
}

void
Chipset::initMem() {
  int i = 0;
  try {
    Bloat biosLoad = loadBiosFromFile("bios.bin");
    for (i = 0; i < biosLoad.size(); i++) {
      mainMem[i] = biosLoad[i];
    }
  } catch (WrongFileException) {
    printf("Failed to load bios from file\n");
    for (i = 0; !(bios[i-1] == HALT && (bios[i] == 0 || bios[i+1] == 0)); i++) {
      mainMem[i] = bios[i];
    }
  }
  for (; i < maxMem; i++) {
    mainMem[i] = 0;
  }
}

void
Chipset::startClock() {
  printf("Starting execution\n");

  int result = 0;
  do {
    result = cpu.coreStep();
    usleep(100);

    if (result == REBOOT) {
      initMem();
      cpu.init();
    }
  } while (result != HALT);

  printf("Exiting\n");
}


void
Chipset::addComponent(const Component& comp) {
  components.push_back(comp);
}

//void
//Chipset::addCpu(Cpu & _cpu) {
//  cpu = _cpu;
//}

const Cpu&
Chipset::getCpu(int num) const {
  /* for now num is just an API placeholder */
  return cpu;
}

void
Chipset::singlePutToComponent(const int& numComp, const int& signal) {
  components[numComp].put(signal);
}

int
Chipset::singleGetFromComponent(const int& numComp) {
  components[numComp].get();
}
