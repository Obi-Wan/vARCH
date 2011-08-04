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

Chipset::Chipset(const uint32_t& _maxMem, int32_t * _mainMem)
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
//  MOV + ARG_1(CONST) + ARG_2(REG), 10, REG_DATA_1,
//  MOV + ARG_1(CONST) + ARG_2(REG), 20, REG_DATA_2,
//  ADD + ARG_1(REG) + ARG_2(REG), REG_DATA_1, REG_DATA_2,
//  MOV + ARG_1(REG) + ARG_2(ADDR), REG_DATA_2, 16,
//  HALT, 0, 0 };

const int32_t
Chipset::bios[] = {
  MOV   + ARG_1(CONST) + ARG_2(REG), 10          , REG_DATA_1,
  MOV   + ARG_1(CONST) + ARG_2(REG), 20          , REG_DATA_2,
  ADD   + ARG_1(REG)  + ARG_2(REG), REG_DATA_1  , REG_DATA_2,
  PUSH  + ARG_1(REG)              , REG_DATA_2  ,
  HALT, 0, 0 };

//const int32_t
//Chipset::bios[] = {
//  MOV + ARG_1(CONST) + ARG_2(REG),                   7,          REG_DATA_1,
//  MOV + ARG_1(CONST) + ARG_2(REG),                   1,          REG_DATA_2,
//  IFEQJ + ARG_1(CONST) + ARG_2(REG) + ARG_3(CONST),   17,         REG_DATA_1, 0,
//  MULT + ARG_1(REG_POST_DECR) + ARG_2(REG),         REG_DATA_1, REG_DATA_2,
//  IFNEQJ + ARG_1(CONST) + ARG_2(REG) + ARG_3(CONST),  10,         REG_DATA_1, 0,
//  MOV + ARG_1(REG) + ARG_2(ADDR),                   REG_DATA_2, 23,
//  HALT, 0, 0 };

inline Bloat
Chipset::loadBiosFromFile(const char * file) {
  BinLoader handler(file);
  return handler.getBinFileContent();
}

inline void
Chipset::initMem() {
  uint32_t i = 0;
  try {
    Bloat biosLoad = loadBiosFromFile("bios.bin");

    CHECK_THROW(biosLoad.size() < maxMem,
        WrongFileException("Bios Too Big! It doesn't fit memory!"));

    for (i = 0; i < biosLoad.size(); i++) {
      mainMem[i] = biosLoad[i];
    }
  } catch (const WrongFileException & e) {
    printf("Failed to load bios from file:\n%s\n", e.what());
    for (i = 0; !(bios[i-1] == HALT && (bios[i] == 0 || bios[i+1] == 0)); i++) {
      mainMem[i] = bios[i];
    }
  }
  for (; i < maxMem; i++) {
    mainMem[i] = 0;
  }
}

void
Chipset::startClock()
{
  DebugPrintf(("Starting execution\n"));
  uint32_t timeOfExecution = 0;

  int result = 0;
  do {

    // Let's trigger events
    for (size_t i = 0; i < components.size(); i++) {
      components[i]->checkInterruptEvents();
    }

    // Let's execute the next istruction
    result = cpu.coreStep();

    const uint32_t waitTime = 100 + cpu.getTimeDelay();
    timeOfExecution += waitTime;
    // Sleep for a while
    usleep(waitTime);

    // Reset if REBOOT istruction got.
    if (result == REBOOT) {
      initMem();
      cpu.init();
    }
  } while (result != HALT);

  DebugPrintf(("Exiting\n"));
  printf("\nExecution time (in pseudo-microseconds): %u\n\n", timeOfExecution);
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
Chipset::getCpu(const uint32_t & num) const {
  /* for now num is just an API placeholder */
  return cpu;
}

void
Chipset::singlePutToComponent(const uint32_t& signal, const uint32_t& arg) {
  DebugPrintf(("Signal  in dec %d in ex %x\n", signal, signal));
  uint16_t request = EXTRACT_REQUEST(signal);
  DebugPrintf(("Request in dec %d in ex %x\n", request, request));
  uint16_t device  = EXTRACT_DEVICE(signal);
  DebugPrintf(("Device in dec %d in ex %x\n", device, device));
  if (device > components.size()) {
    WarningPrintf(("accessing a non existent component: %d\n", device));
  } else {
    components[device]->put(request, arg);
  }
}

int
Chipset::singleGetFromComponent(const uint32_t& numComp) {
  if ( ((uint32_t)numComp) > components.size()) {
    WarningPrintf(("accessing a non existent component: %d\n", numComp));
    return DATA_READY_ERROR;
  } else {
    return components[numComp]->get();
  }
}
