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

Cpu::Cpu(Chipset& _chipset, Mmu& mC)
        : chipset(_chipset)
        , memoryController(mC)
{
  init();
}

//Cpu::Cpu(const Cpu& orig) { }

Cpu::~Cpu() { }

void
Cpu::init() {
  progCounter = 0;
  resetFlags(flags);
  resetRegs();
}

/** Writes to standard output the content of registers, program counter
 * and the content of the first region of memory
 */
void
Cpu::dumpRegistersAndMemory() const {
  printf("Data Registers:");
  for( int i = 0; i < NUM_REGS; i++) {
    printf(" %d", regsData[i]);
  }
  
//  printf("\nAddress Registers:");
//  for (int i = 0; i < 8; i++) {
//    printf(" %d", regsAddr[i]);
//  }

  printf("\nProgram Counter: %d\n",progCounter);

  for( int i = 0; i < memoryController.getMaxMem(); i++) {
    printf("Mem: %d\tData: %d\n", i, memoryController.loadFromMem(i));
  }
}

int
Cpu::coreStep() {

  Flags newFlags;
  resetFlags(newFlags);
  int currentIstr = memoryController.loadFromMem(progCounter++);

  printf("nuova istruzione %d: num args: %d\n", progCounter,
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
Cpu::istructsZeroArg(const int& istr, Flags& newFlags) throw(WrongIstructionException) {
  switch (istr) {
    case SLEEP:
    case REBOOT:
    case HALT:
      return istr;
      
    case PUSHA:
    case POPA:
    case RET:
      break;

    default:
      throw new WrongIstructionException();
      break;
  }
  return 0;
}

int
Cpu::istructsOneArg(const int& istr, Flags& newFlags) throw(WrongIstructionException) {

  int typeArg = GET_ARG_1(istr);
  int arg = memoryController.loadFromMem(progCounter++);
  int temp = loadArg(arg, typeArg);

  const int polishedIstr = istr - ARG_1(typeArg);

  switch (polishedIstr) {
    case NOT:
      temp = ~temp;
      break;
    case INCR:
      temp++;
      break;
    case DECR:
      temp--;
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
    case PUSH:
    case POP:
      break;
      
    case IFJ:
      if (flags.zero) progCounter = temp;
      break;
    case IFNJ:
      if (!flags.zero) progCounter = temp;
      break;
    case JMP:
      progCounter = temp;
      break;
    case JSR:
      break;
      
    default:
      throw new WrongIstructionException();
      break;
  }

  if (polishedIstr < STACK || polishedIstr == POP) {
    storeArg(arg, typeArg, temp);
  }

  return 0;
}

int
Cpu::istructsTwoArg(const int& istr, Flags& newFlags) throw(WrongIstructionException) {

  int typeArg1 = GET_ARG_1(istr);
  int arg1 = memoryController.loadFromMem(progCounter++);
  int temp1 = loadArg(arg1, typeArg1);

  int typeArg2 = GET_ARG_2(istr);
  int arg2 = memoryController.loadFromMem(progCounter++);
  int temp2 = loadArg(arg2, typeArg2);

  const int polishedIstr = istr - ARG_1(typeArg1) - ARG_2(typeArg2);

  switch (polishedIstr) {
    case MOV:
      temp2 = temp1;
      break;
    case ADD:
      temp2 += temp1;
      break;
    case MULT:
      temp2 *= temp1;
      break;
    case SUB:
      temp2 -= temp1;
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
      break;

    case PUT:
      chipset.singlePutToComponent(temp2,temp1);
      break;
    case GET:
      temp2 = chipset.singleGetFromComponent(temp1);
      break;

    case EQ:
      if (temp1 == temp2) newFlags.zero = true;
      break;
    case LO:
      if (temp1 < temp2) newFlags.zero = true;
      break;
    case MO:
      if (temp1 > temp2) newFlags.zero = true;
      break;
    case LE:
      if (temp1 <= temp2) newFlags.zero = true;
      break;
    case ME:
      if (temp1 >= temp2) newFlags.zero = true;
      break;
    case NEQ:
      if (temp1 != temp2) newFlags.zero = true;
      break;
      
    default:
      throw new WrongIstructionException();
      break;
  }

  if (polishedIstr <= XOR || polishedIstr == GET) {
    
    storeArg(arg2, typeArg2, temp2); /* Operations that do modify args */

    if (typeArg1 & 4) { /* in case the first arg was modified */
      storeArg(arg1, typeArg1, temp1);
    }
  }

  return 0;
}

int
Cpu::istructsThreeArg(const int& istr, Flags& newFlags) throw(WrongIstructionException) {

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
      throw new WrongIstructionException();
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
      return memoryController.loadFromMem(regsData[arg]);
      break;

    case REG_PRE_INCR:
      return ++regsData[arg];
      break;
    case REG_PRE_DECR:
      return --regsData[arg];
      break;
    case REG:
    case REG_POST_INCR:
    case REG_POST_DECR:
      return regsData[arg];
    default:
      throw new WrongArgumentException();
      break;
  }
}

void
Cpu::storeArg(const int& arg, const int& typeArg, int value) throw(WrongArgumentException) {
  switch (typeArg) {
    case COST:
      break;
    case ADDR:
      memoryController.storeToMem(value, arg);
      break;
    case ADDR_IN_REG:
      memoryController.storeToMem(value, regsData[arg]);
      break;

    case REG:
    case REG_PRE_INCR:
    case REG_PRE_DECR:
      regsData[arg] = value;
      break;
    case REG_POST_INCR:
      regsData[arg] = ++value;
      break;
    case REG_POST_DECR:
      regsData[arg] = --value;
      break;
    default:
      throw new WrongArgumentException();
      break;
  }
}