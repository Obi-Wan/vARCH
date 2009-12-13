/* 
 * File:   Cpu.cpp
 * Author: ben
 * 
 * Created on 19 agosto 2009, 15.51
 */

#include "Cpu.h"
#include "Chipset.h"
#include "../include/std_istructions.h"
#include "../include/asm_helpers.h"
#include "../include/macros.h"

#include <cstdio>

#define SET_ARITM_FLAGS( x ) ( x < 0) ? F_NEGATIVE : ( (! x ) ? F_ZERO : 0 )

Cpu::Cpu(Chipset& _chipset, Mmu& mC)
        : chipset(_chipset)
        , memoryController(mC)
        , sP(*this)
{
  init();
}

//Cpu::Cpu(const Cpu& orig) { }

Cpu::~Cpu() { }

void
Cpu::init() {
  progCounter = 0;
  resetFlags(flags);
  flags += F_SVISOR | INT_PUT( INT_MAX_S_PR ); // start in supervisor mode
  resetRegs();
}

/** Writes to standard output the content of registers, program counter
 * and the content of the first region of memory
 */
void
Cpu::dumpRegistersAndMemory() const {

//  int old = setFlags(F_SVISOR);

  printf("Data Registers:");
  for( int i = 0; i < NUM_REGS; i++) {
    printf(" %d", regsData[i]);
  }
  
  printf("\nAddress Registers:");
  for (int i = 0; i < 8; i++) {
    printf(" %d", regsAddr[i]);
  }

  printf("\nProgram Counter: %d\n",progCounter);

  printf("Stack pointers, user: %d \tsupervisor: %d\n", sP.getUStackPointer(),
         sP.getStackPointer());

  for( int i = 0; i < memoryController.getMaxMem(); i++) {
    printf("Mem: %d\tData: %d\n", i, memoryController.loadFromMem(i));
  }

//  restoreFlags(old);
}

int
Cpu::coreStep() {

  //DebugPrintf(("Processing interrupts to see if they can stop execution\n"));
  // Let's now consider interrupts
  if (chipset.hasInterruptReady()) {
    const InterruptHandler::InterruptsRecord &intRecord = chipset.getTopInterrupt();
    if (intRecord.getPriority() > INT_GET(flags)) {
      DebugPrintf(("we got an interrupt!"));
      int currentFlags = flags;
      flags += F_SVISOR;
      sP.push(currentFlags);
      sP.push(progCounter);

      progCounter = memoryController.loadFromMem(regsAddr[7] +
              2 * intRecord.getDeviceId());
      restoreFlags( memoryController.loadFromMem(
              regsAddr[7] + 2 * intRecord.getDeviceId() + 1) );

      chipset.topInterruptServed();
    }
  }

  //DebugPrintf(("Proceding with instructions execution\n"));
  // Now proced with instruction execution
  int newFlags = flags;
  resetFlags(newFlags);
  int currentIstr = memoryController.loadFromMem(progCounter++);

  DebugPrintf(("Istruzione nell'area di mem: %d num args: %d\n", progCounter-1,
         (currentIstr >> 30 ) & 3));

  int res = 0;

  try {
    switch ((currentIstr >> 30 ) & 3) {
      case 0:
        res = istructsZeroArg(currentIstr, newFlags);
        break;
      case 1:
        res = istructsOneArg(currentIstr, newFlags);
        break;
      case 2:
        res = istructsTwoArg(currentIstr, newFlags);
        break;
      case 3:
        res = istructsThreeArg(currentIstr, newFlags);
        break;
    }
  } catch (WrongArgumentException e) {
    printf("Wrong argument of the istruction. %s\n", e.what());
  } catch (WrongIstructionException e) {
    printf("Wrong istruction. %s\n", e.what());
  }
  flags = newFlags;
  return res;
}

int
Cpu::istructsZeroArg(const int& istr, int& newFlags) throw(WrongIstructionException) {
  switch (istr) {
    case SLEEP:
      return istr;
    case REBOOT:
    case HALT:
      return (flags & F_SVISOR) ? istr : SLEEP;
      
    case PUSHA:
      sP.pushAllRegs();
      break;
    case POPA:
      sP.popAllRegs();
      break;
    case RET:
      progCounter = sP.pop();
      break;
    case RETEX:
      flags += F_SVISOR;
      progCounter = sP.pop();
      newFlags = sP.pop();
      break;

    default:
      throw WrongIstructionException();
      break;
  }
  return 0;
}

int
Cpu::istructsOneArg(const int& istr, int& newFlags) throw(WrongIstructionException) {

  int typeArg = GET_ARG_1(istr);
  DebugPrintf(("Loaded arg 1: 0x%X\n", typeArg));
  int arg = memoryController.loadFromMem(progCounter++);
  int temp = loadArg(arg, typeArg);

  const int polishedIstr = istr - ARG_1(typeArg);

  int old = temp;
  switch (polishedIstr) {
    case NOT:
      temp = ~temp;
      break;
    case INCR:
      if (old > ++temp) flags += F_OVERFLOW;
      break;
    case DECR:
      if (old < --temp) flags += F_OVERFLOW;
      break;
    case COMP2:
      temp = -temp;
      break;
    case LSH:
      temp <<= 1;
      break;
    case RSH:
      temp >>= 1;
      break;

    case STACK:
      sP.setStackPointer(temp);
      break;
    case PUSH:
      sP.push(temp);
      break;
    case POP:
      temp = sP.pop();
      break;
      
    case IFJ:
      if (flags & F_ZERO) progCounter = temp;
      break;
    case IFNJ:
      if (! (flags & F_ZERO)) progCounter = temp;
      break;
    case JMP:
      progCounter = temp;
      break;
    case JSR:
      sP.push(progCounter);
      progCounter = temp;
      break;
    case TCJ:
      if (flags & F_CARRY) progCounter = temp;
      break;
    case TZJ:
      if (flags & F_ZERO) progCounter = temp;
      break;
    case TOJ:
      if (flags & F_OVERFLOW) progCounter = temp;
      break;
    case TNJ:
      if (flags & F_NEGATIVE) progCounter = temp;
      break;
    case TSJ:
      if (flags & F_SVISOR) progCounter = temp;
      break;
      
    default:
      throw WrongIstructionException();
      break;
  }

  if (polishedIstr < STACK || polishedIstr == POP) {
    newFlags += SET_ARITM_FLAGS(temp);
    storeArg(arg, typeArg, temp);
  }

  return 0;
}

int
Cpu::istructsTwoArg(const int& istr, int& newFlags) throw(WrongIstructionException) {

  int typeArg1 = GET_ARG_1(istr);
  DebugPrintf(("Loaded arg 1: 0x%X\n", typeArg1));
  int arg1 = memoryController.loadFromMem(progCounter++);
  int temp1 = loadArg(arg1, typeArg1);

  int typeArg2 = GET_ARG_2(istr);
  DebugPrintf(("Loaded arg 2: 0x%X\n", typeArg2));
  int arg2 = memoryController.loadFromMem(progCounter++);
  int temp2 = loadArg(arg2, typeArg2);

  const int polishedIstr = istr - ARG_1(typeArg1) - ARG_2(typeArg2);

  int old = temp2;
  switch (polishedIstr) {
    case MOV:
      temp2 = temp1;
      break;
    case ADD:
      temp2 += temp1;
      if (old > temp2) flags += F_OVERFLOW;
      break;
    case MULT:
      temp2 *= temp1;
      if (old > temp2) flags += F_OVERFLOW;
      break;
    case SUB:
      temp2 -= temp1;
      if (old < temp2) flags += F_OVERFLOW;
      break;
    case DIV:
      temp2 /= temp1;
      break;
    case QUOT:
      temp2 %= temp1;
      break;

    case AND:
      temp2 &= temp1;
      break;
    case OR:
      temp2 |= temp1;
      break;
    case XOR:
      temp2 ^= temp1;
      break;

    case MMU:
      if (flags & F_SVISOR) memoryController.resetLimits(temp1, temp2);
      break;

    case PUT:
      chipset.singlePutToComponent(temp2,temp1);
      break;
    case GET:
      temp2 = chipset.singleGetFromComponent(temp1);
      break;

    case EQ:
      if (temp1 == temp2) newFlags += F_ZERO;
      break;
    case LO:
      if (temp1 < temp2) newFlags += F_ZERO;
      break;
    case MO:
      if (temp1 > temp2) newFlags += F_ZERO;
      break;
    case LE:
      if (temp1 <= temp2) newFlags += F_ZERO;
      break;
    case ME:
      if (temp1 >= temp2) newFlags += F_ZERO;
      break;
    case NEQ:
      if (temp1 != temp2) newFlags += F_ZERO;
      break;
      
    default:
      throw WrongIstructionException();
      break;
  }

  if (polishedIstr <= XOR || polishedIstr == GET) {

    newFlags += SET_ARITM_FLAGS(temp2);
    storeArg(arg2, typeArg2, temp2); /* Operations that do modify args */

    if (typeArg1 & 4) { /* in case the first arg was modified */
      storeArg(arg1, typeArg1, temp1);
    }
  }

  return 0;
}

int
Cpu::istructsThreeArg(const int& istr, int& newFlags) throw(WrongIstructionException) {

  int typeArg1 = GET_ARG_1(istr);
  int arg1 = memoryController.loadFromMem(progCounter++);
  int temp1 = loadArg(arg1, typeArg1);

  int typeArg2 = GET_ARG_2(istr);
  int arg2 = memoryController.loadFromMem(progCounter++);
  int temp2 = loadArg(arg2, typeArg2);

  int typeArg3 = GET_ARG_3(istr);
  int arg3 = memoryController.loadFromMem(progCounter++);
  int temp3 = loadArg(arg3, typeArg3);

  const int polishedIstr = istr - ARG_1(typeArg1) - ARG_2(typeArg2)
                                - ARG_3(typeArg3);

  switch (polishedIstr) {
    case BPUT:
      break;
    case BGET:
      break;
    case IFEQJ:
      if (temp2 == temp3) progCounter = temp1;
      break;
    case IFNEQJ:
      if (temp2 != temp3) progCounter = temp1;
      break;
    case IFLOJ:
      if (temp2 < temp3) progCounter = temp1;
      break;
    case IFMOJ:
      if (temp2 > temp3) progCounter = temp1;
      break;
    case IFLEJ:
      if (temp2 <= temp3) progCounter = temp1;
      break;
    case IFMEJ:
      if (temp2 >= temp3) progCounter = temp1;
      break;

    default:
      throw WrongIstructionException();
      break;
  }
  return 0;
}

int
Cpu::loadArg(const int& arg,const int& typeArg) throw(WrongArgumentException) {
  const int relative = (typeArg & 0x10) ? (progCounter -1) : 0;
  DebugPrintf(("Relative: %d\n", relative));
  switch (typeArg & 0xf) {
    case COST:
      DebugPrintf(("COST: %d\n", arg));
      return (arg + relative);
//    case ADDR:
//      DebugPrintf(("ADDR: %d\n", arg));
//      return memoryController.loadFromMem(arg + relative);
//    case ADDR_IN_REG:
//      DebugPrintf(("ADDR_IN_REG: %d\n", arg));
//      return memoryController.loadFromMem(getReg(arg) + relative);

    case REG_PRE_INCR:
      DebugPrintf(("REG_PRE_INCR: %d\n", arg));
      return (getReg(arg) +1);
    case REG_PRE_DECR:
      DebugPrintf(("REG_PRE_DECR: %d\n", arg));
      return (getReg(arg) -1);
    case REG:
    case REG_POST_INCR:
    case REG_POST_DECR:
      DebugPrintf(("REG / REG_POST_*: %d\n", arg));
      return getReg(arg);

    case ADDR_PRE_INCR:
      DebugPrintf(("ADDR_PRE_INCR: %d\n", arg));
      return (memoryController.loadFromMem(arg + relative) +1);
    case ADDR_PRE_DECR:
      DebugPrintf(("ADDR_PRE_DECR: %d\n", arg));
      return (memoryController.loadFromMem(arg + relative) -1);
    case ADDR:
    case ADDR_POST_INCR:
    case ADDR_POST_DECR:
      DebugPrintf(("ADDR / ADDR_POST_*: %d\n", arg));
      return memoryController.loadFromMem(arg + relative);

    case ADDR_IN_REG_PRE_INCR:
      DebugPrintf(("ADDR_IN_REG_PRE_INCR: %d\n", arg));
      return (memoryController.loadFromMem(getReg(arg) + relative) +1);
    case ADDR_IN_REG_PRE_DECR:
      DebugPrintf(("ADDR_IN_REG_PRE_DECR: %d\n", arg));
      return (memoryController.loadFromMem(getReg(arg) + relative) -1);
    case ADDR_IN_REG:
    case ADDR_IN_REG_POST_INCR:
    case ADDR_IN_REG_POST_DECR:
      DebugPrintf(("ADDR_IN_REG / ADDR_IN_REG_POST_*: %d\n", arg));
      return memoryController.loadFromMem(getReg(arg) + relative);
      
    default:
      throw WrongArgumentException("Failed in loading");
      break;
  }
}

void
Cpu::storeArg(const int& arg, const int& typeArg, int value) throw(WrongArgumentException) {
  const int relative = (typeArg & 0x10) ? (progCounter -1) : 0;
  DebugPrintf(("Relative: %d\n", relative));
  switch (typeArg & 0xf) {
//    case ADDR:
//      DebugPrintf(("ADDR: %d\n", arg));
//      memoryController.storeToMem(value, arg + relative);
//      break;
//    case ADDR_IN_REG:
//      DebugPrintf(("ADDR_IN_REG: %d\n", arg));
//      memoryController.storeToMem(value, getReg(arg) + relative);
//      break;

    case REG:
    case REG_PRE_INCR:
    case REG_PRE_DECR:
      DebugPrintf(("REG / REG_PRE_*: %d\n", arg));
      setReg(arg, value);
      break;
    case REG_POST_INCR:
      DebugPrintf(("REG_POST_INCR: %d\n", arg));
      setReg(arg, ++value);
      break;
    case REG_POST_DECR:
      DebugPrintf(("REG_POST_DECR: %d\n", arg));
      setReg(arg, --value);
      break;

    case ADDR:
    case ADDR_PRE_INCR:
    case ADDR_PRE_DECR:
      DebugPrintf(("ADDR / ADDR_PRE_*: %d\n", arg));
      memoryController.storeToMem(value, arg + relative);
      break;
    case ADDR_POST_INCR:
      DebugPrintf(("ADDR_POST_INCR: %d\n", arg));
      memoryController.storeToMem(++value, arg + relative);
      break;
    case ADDR_POST_DECR:
      DebugPrintf(("ADDR_POST_DECR: %d\n", arg));
      memoryController.storeToMem(--value, arg + relative);
      break;

    case ADDR_IN_REG:
    case ADDR_IN_REG_PRE_INCR:
    case ADDR_IN_REG_PRE_DECR:
      DebugPrintf(("ADDR_IN_REG / ADDR_IN_REG_PRE_*: %d\n", arg));
      memoryController.storeToMem(value, getReg(arg) + relative);
      break;
    case ADDR_IN_REG_POST_INCR:
      DebugPrintf(("ADDR_IN_REG_POST_INCR: %d\n", arg));
      memoryController.storeToMem(++value, getReg(arg) + relative);
      break;
    case ADDR_IN_REG_POST_DECR:
      DebugPrintf(("ADDR_IN_REG_POST_DECR: %d\n", arg));
      memoryController.storeToMem(--value, getReg(arg) + relative);
      break;
    default:
      throw WrongArgumentException("Failed in storing");
      break;
  }
}

inline int
Cpu::getReg(const int& arg) {
  int type = arg / OFFSET_REGS;
  DebugPrintf(("arg: %d type: %d, spec: %d\n",arg,type,arg % OFFSET_REGS));
  switch (type) {
    case DATA_REGS:
      return regsData[ arg % OFFSET_REGS ];
    case ADDR_REGS:
      return regsAddr[ arg % OFFSET_REGS ];
    case STCK_PTRS:
      if (arg == USER_STACK_POINTER) {
        return (flags & F_SVISOR) ? sP.getUStackPointer() : sP.getStackPointer();
      } else if (arg == STACK_POINTER) {
        return sP.getStackPointer();
      }
      throw WrongArgumentException("No such stack pointer");
    case STATE_REGISTER:
      return (flags & F_SVISOR) ? flags : int(flags);
    default:
      throw WrongArgumentException("No such register");
  }
}


inline void
Cpu::setReg(const int& arg, const int& value) {
  int type = arg / OFFSET_REGS;
  DebugPrintf( ("arg: %d type: %d, spec: %d\n",arg,type,arg % OFFSET_REGS) );
  switch (type) {
    case DATA_REGS:
      regsData[ arg % OFFSET_REGS ] = value;
      break;
    case ADDR_REGS:
      regsAddr[ arg % OFFSET_REGS ] = value;
      break;
    case STCK_PTRS:
      if (arg == USER_STACK_POINTER) {
        if (flags & F_SVISOR) {
          sP.setUStackPointer(value);
        } else {
          sP.setStackPointer(value);
        }
        break;
      } else if (arg == STACK_POINTER) {
        sP.setStackPointer(value);
        break;
      }
      throw WrongArgumentException("No such stack pointer");
    case STATE_REGISTER:
      if (flags & F_SVISOR) {
        flags = value;
        break;
      } else {
        throw WrongArgumentException("Not Allowed to modify SR");
      }
    default:
      throw WrongArgumentException("No such register");
  }
}
