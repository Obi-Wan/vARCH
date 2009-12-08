/* 
 * File:   Chipset.cpp
 * Author: ben
 * 
 * Created on 20 agosto 2009, 14.51
 */

#include <unistd.h>

#include "Chipset.h"
#include "../include/std_istructions.h"
#include "../include/asm_helpers.h"
#include "SystemTimer.h"
#include "CharTerminal.h"
#include "macros.h"

Chipset::Chipset(const int& _maxMem, int * _mainMem)
    : cpu(* (new Cpu(*this,*(new Mmu(_maxMem,_mainMem))))), mainMem(_mainMem)
      , maxMem(_maxMem) {

  // let's init memory, and print some debug info
  initMem();
  cpu.dumpRegistersAndMemory();

  // next we add the system timer to the components connected to the chipset
  components.push_back( new SystemTimer( SystemTimer::TIMER_0250_HZ |
                                     SystemTimer::TIMER_0100_HZ    ));
  // then we add a charterminal
  components.push_back( new CharTerminal() );
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
  STACK + ARG_1(COST)             , 20          ,
  MOV   + ARG_1(COST) + ARG_2(REG), 10          , REG_DATA_1,
  MOV   + ARG_1(COST) + ARG_2(REG), 20          , REG_DATA_2,
  ADD   + ARG_1(REG)  + ARG_2(REG), REG_DATA_1  , REG_DATA_2,
  PUSH  + ARG_1(REG)              , REG_DATA_2  ,
  HALT, 0, 0 };

//const int
//Chipset::bios[] = {
//  MOV + ARG_1(COST) + ARG_2(REG),                   7,          REG_DATA_1,
//  MOV + ARG_1(COST) + ARG_2(REG),                   1,          REG_DATA_2,
//  IFEQJ + ARG_1(COST) + ARG_2(REG) + ARG_3(COST),   17,         REG_DATA_1, 0,
//  MULT + ARG_1(REG_POST_DECR) + ARG_2(REG),         REG_DATA_1, REG_DATA_2,
//  IFNEQJ + ARG_1(COST) + ARG_2(REG) + ARG_3(COST),  10,         REG_DATA_1, 0,
//  MOV + ARG_1(REG) + ARG_2(ADDR),                   REG_DATA_2, 23,
//  HALT, 0, 0 };

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
    WarningPrintf(("Failed to load bios from file\n"));
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
  DebugPrintf(("Starting execution\n"));

  int result = 0;
  do {

    // Let's trigger events
    for (unsigned int i = 0; i < components.size(); i++) {
      components[i]->checkInterruptEvents();
    }

    // Let's execute the next istruction
    result = cpu.coreStep();
    // Sleep for a while
    usleep(100);

    // Reset if REBOOT istruction got.
    if (result == REBOOT) {
      initMem();
      cpu.init();
    }
  } while (result != HALT);

  DebugPrintf(("Exiting\n"));
}


void
Chipset::addComponent(Component* comp) {
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
  if (numComp > components.size()) {
    WarningPrintf(("accessing a non existent component"));
  } else {
    components[numComp]->put(signal);
  }
}

int
Chipset::singleGetFromComponent(const int& numComp) {
  if (numComp > components.size()) {
    WarningPrintf(("accessing a non existent component"));
  } else {
    components[numComp]->get();
  }
}
