/* 
 * File:   Chipset.cpp
 * Author: ben
 * 
 * Created on 20 agosto 2009, 14.51
 */

#include <unistd.h>

#include "std_istructions.h"
#include "macros.h"

#include "Chipset.h"
#include "CharTerminal.h"

Chipset::Chipset(const uint32_t & _maxMem, DoubleWord * _mainMem)
    : cpu(* (new Cpu(*this,*(new Mmu(_maxMem,_mainMem))))), mainMem(_mainMem)
    , maxMem(_maxMem)
{
  // let's init memory, and print some debug info
  initMem();
  cpu.dumpRegistersAndMemory();

  components.push_back( this );
  // then we add a charterminal
  components.push_back( new CharTerminal() );
}

inline Bloat
Chipset::loadBiosFromFile(const char * file)
{
  BinLoader handler(file);
  return handler.getBinFileContent();
}

void
Chipset::put(const ComponentRequestType & request, const int32_t & arg) {
  switch (request) {
    case ComponentRequestType::COMP_TYPE:
      simpleUnsafeResponse = static_cast<int32_t>(ComponentType::COMP_CHAR);
      dataReady = DataReady::DATA_READY_TRUE;
      break;
    case ComponentRequestType::COMP_GET_FEATURES:
      switch (arg) {
        case 0:
          simpleUnsafeResponse = static_cast<int32_t>(this->components.size());
          break;
      }
      dataReady = DataReady::DATA_READY_TRUE;
      break;
    case ComponentRequestType::COMP_SET_FEATURES:
      dataReady = DataReady::DATA_READY_ERROR;
      break;
  }
}

void
Chipset::initMem()
{
  uint32_t block = 0;
  for (; block < maxMem / 4; block++)
  {
    mainMem[block].u32 = 0;
  }
  try {
    Bloat biosLoad = loadBiosFromFile("bios.bin");

    CHECK_THROW(biosLoad.size() < maxMem,
        WrongFileException("Bios Too Big! It doesn't fit in memory!"));

    const size_t blocksNum = biosLoad.size() / 4;
    for (block = 0; block < blocksNum; block++)
    {
      mainMem[block].u32 = *((uint32_t *)&biosLoad[block*4]);
    }
    const size_t remaining = biosLoad.size() - blocksNum * 4;
    for (size_t count = 0; count < remaining; count++)
    {
      mainMem[block].u8[count] = biosLoad[block*4 + count];
    }
  } catch (const WrongFileException & e) {
    printf("Failed to load bios from file:\n%s\n", e.what());
    printf("Exiting.\n");
    throw e;
  }
}

void
Chipset::startClock()
{
  DebugPrintf(("Starting execution\n"));
  uint32_t timeOfExecution = 0;

  StdInstructions result = SLEEP;
  do {
    // Let's execute the next istruction
    try {
      result = cpu.coreStep();
    } catch (const BasicException & e) {
      printf("\n\n -> EXECUTION ERROR! <-\n%s", e.what());
      throw(e);
    }

#ifdef DEBUG
    cpu.dumpRegistersAndMemory();
#endif

    const uint32_t waitTime = 100 + cpu.getTimeDelay();
    timeOfExecution += waitTime;
    // Sleep for a while
    usleep(waitTime);

    // Reset if REBOOT istruction got.
    if (result == REBOOT)
    {
      initMem();
      cpu.init();
    }
  } while (result != HALT);

  printf("\nExecution time (in pseudo-microseconds): %u\n\n", timeOfExecution);
}

void
Chipset::singlePutToComponent(const uint32_t & signal, const uint32_t & arg)
{
  size_t device = static_cast<size_t>(EXTRACT_DEVICE(signal));

  Component::ComponentRequestType request =
      static_cast<Component::ComponentRequestType>(EXTRACT_REQUEST(signal));

#ifdef DEBUG
  deb_sig = static_cast<uint32_t>(signal);
  deb_req = static_cast<uint32_t>(request);
  deb_dev = static_cast<uint32_t>(device);
  DebugPrintf(("Signal  in dec %u in ex %x\n", deb_sig, deb_sig));
  DebugPrintf(("Request in dec %u in ex %x\n", deb_req, deb_req));
  DebugPrintf(("Device  in dec %u in ex %x\n", deb_dev, deb_dev));
#endif

  if ( ((size_t)device) >= components.size()) {
    std::string error_msg = std::string("accessing a non existent component: ")
                            + std::to_string(device);
    InfoPrintf(("%s\n", error_msg.c_str()));
    throw WrongComponentException(error_msg);
  }
  components[device]->put(request, arg);
}

int32_t
Chipset::singleGetFromComponent(const uint32_t & numComp)
{
  if ( ((size_t)numComp) > components.size())
  {
    std::string error_msg = std::string("accessing a non existent component: ") + std::to_string(numComp);
    WarningPrintf(("%s\n", error_msg.c_str()));
    throw WrongComponentException(error_msg);
  }
  return components[numComp]->get();
}
