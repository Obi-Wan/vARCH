/* 
 * File:   Cpu.cpp
 * Author: ben
 * 
 * Created on 19 agosto 2009, 15.51
 */

#include "Cpu.h"
#include "../include/std_istructions.h"
#include "../include/asm_helpers.h"

#include <stdio.h>

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
  flags += F_SVISOR; // start in supervisor mode
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

  int newFlags = flags;
  resetFlags(newFlags);
  int currentIstr = memoryController.loadFromMem(progCounter++);

  printf("Istruzione nell'area di mem: %d num args: %d\n", progCounter-1,
         (currentIstr >> 30 ) & 3);

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
      sP.pushAll();
      break;
    case POPA:
      sP.popAll();
      break;
    case RET:
      progCounter = sP.pop();
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
  int arg1 = memoryController.loadFromMem(progCounter++);
  int temp1 = loadArg(arg1, typeArg1);

  int typeArg2 = GET_ARG_2(istr);
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
  switch (typeArg) {
    case COST:
      return arg;
      break;
    case ADDR:
      return memoryController.loadFromMem(arg);
      break;
    case ADDR_IN_REG:
      return memoryController.loadFromMem(getReg(arg));
      break;

    case REG_PRE_INCR:
      return (getReg(arg) +1);
      break;
    case REG_PRE_DECR:
      return (getReg(arg) +1);
      break;
    case REG:
    case REG_POST_INCR:
    case REG_POST_DECR:
      return getReg(arg);
    default:
      throw WrongArgumentException("Failed in loading");
      break;
  }
}

void
Cpu::storeArg(const int& arg, const int& typeArg, int value) throw(WrongArgumentException) {
  switch (typeArg) {
    case ADDR:
      memoryController.storeToMem(value, arg);
      break;
    case ADDR_IN_REG:
      memoryController.storeToMem(value, getReg(arg));
      break;

    case REG:
    case REG_PRE_INCR:
    case REG_PRE_DECR:
      setReg(arg, value);
      break;
    case REG_POST_INCR:
      setReg(arg, ++value);
      break;
    case REG_POST_DECR:
      setReg(arg, --value);
      break;
    default:
      throw WrongArgumentException("Failed in storing");
      break;
  }
}

inline int
Cpu::getReg(const int& arg) {
  int type = arg / OFFSET_REGS;
  int spec = arg % OFFSET_REGS;
  printf("arg: %d type: %d, spec: %d\n",arg,type,spec);
  switch (type) {
    case 0:
      return regsData[spec];
    case 1:
      return regsAddr[spec];
    case 2:
      if (spec) {
        return sP.getStackPointer();
      } else {
        return (flags & F_SVISOR) ? sP.getUStackPointer() : sP.getStackPointer();
      }
    case 3:
      return regsAddr[NUM_REGS-1];
    case 4:
      return (flags & F_SVISOR) ? flags : int(flags);
  }
}


inline void
Cpu::setReg(const int& arg, const int& value) {
  int type = arg / OFFSET_REGS;
  int spec = arg % OFFSET_REGS;
  printf("arg: %d type: %d, spec: %d\n",arg,type,spec);
  switch (type) {
    case 0:
      regsData[spec] = value;
      break;
    case 1:
      regsAddr[spec] = value;
      break;
    case 2:
      if (spec) {
        sP.setStackPointer(value);
      } else {
        if (flags & F_SVISOR) {
          sP.setUStackPointer(value);
        } else {
          sP.setStackPointer(value);
        }
      }
      break;
    case 3:
      regsAddr[NUM_REGS-1] = value;
      break;
    case 4:
      if (flags & F_SVISOR) {
        flags = value;
      } else {
        throw WrongArgumentException();
      }
      break;
  }
}