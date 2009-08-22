/* 
 * File:   Cpu.cpp
 * Author: ben
 * 
 * Created on 19 agosto 2009, 15.51
 */

#include "Cpu.h"
#include "../include/StdIstructions.h"

#include <stdio.h>
#include <exception>

Cpu::Cpu(Mmu & mC) : memoryController(mC), progCounter(0) { }

//Cpu::Cpu(const Cpu& orig) { }

Cpu::~Cpu() { }


/** Writes to standard output the content of registers, program counter
 * and the content of the first region of memory
 */
void
Cpu::dumpRegistersAndMemory() const {
  printf("Data Registers:");
  for (int i = 0; i < 8; i++) {
    printf(" %d", regsData[i]);
  }
  
//  printf("\nAddress Registers:");
//  for (int i = 0; i < 8; i++) {
//    printf(" %d", regsAddr[i]);
//  }

  printf("\nProgram Counter: %d\n",progCounter);

  for (int i = 0; i < memoryController.getMaxMem(); i++) {
    printf("Mem: %d\tData: %d\n", i, memoryController.loadFromMem(i));
  }
}

int
Cpu::coreStep() {
  int currentIstr = memoryController.loadFromMem(progCounter++);

  printf("nuova istruzione %d\n", progCounter);
  
  switch ((currentIstr >> 30 ) & 3) {
    case 0:
      return istructsZeroArg(currentIstr);
    case 1:
      return istructsOneArg(currentIstr);
    case 2:
      return istructsTwoArg(currentIstr);
    case 3:
      return istructsThreeArg(currentIstr);
  }
}

int
Cpu::istructsZeroArg(const int& istr) {
  switch (istr) {
    case SLEEP:
    case REBOOT:
    case HALT:
      return istr;
      
    case PUSH:
    case POP:
    case RET:
      break;

    default:
      throw new std::exception();
      break;
  }
  return 0;
}

int
Cpu::istructsOneArg(const int& istr) {

  int typeArg = (istr >> 23) & 7;
  int arg = memoryController.loadFromMem(progCounter++);
  int temp = loadArg(arg, typeArg);

  switch (istr) {
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
      
    case IFJ:
      if (!zero) break;
    case IFNJ:
      if (zero) break;
    case JMP:
        progCounter = temp;
      break;
      
    default:
      throw new std::exception();
      break;
  }
  
  storeArg(arg, typeArg, temp);

  return 0;
}

int
Cpu::istructsTwoArg(const int& istr) {

  int typeArg1 = (istr >> 23) & 7;
  int arg1 = memoryController.loadFromMem(progCounter++);
  int temp1 = loadArg(arg1, typeArg1);

  int typeArg2 = (istr >> 20) & 7;
  int arg2 = memoryController.loadFromMem(progCounter++);
  int temp2 = loadArg(arg2, typeArg2);

  switch (istr) {

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

    case PUT:
      break;
    case GET:
      break;

    case EQ:
      if (temp1 == temp2) zero = true;
      break;
    case LO:
      if (temp1 < temp2) zero = true;
      break;
    case MO:
      if (temp1 > temp2) zero = true;
      break;
    case LE:
      if (temp1 <= temp2) zero = true;
      break;
    case ME:
      if (temp1 >= temp2) zero = true;
      break;
    case NEQ:
      if (temp1 != temp2) zero = true;
      break;
      
    default:
      throw new std::exception();
      break;
  }

  if (istr < PUT) {
    
    storeArg(arg2, typeArg2, temp2); /* Operations that do modify args */

    if (typeArg1 & 5) { /* in case the first arg was post incr/decr */
      storeArg(arg1, typeArg1, temp1);
    }
  }

  return 0;
}

int
Cpu::istructsThreeArg(const int& istr) {
  switch (istr) {
    case BPUT:
    case BGET:
    case IFEQJ:
    case IFNEQJ:
    case IFLOJ:
    case IFMOJ:
    case IFLEJ:
    case IFMEJ:
      break;

    default:
      throw new std::exception();
      break;
  }
  return 0;
}

int
Cpu::loadArg(const int& arg,const int& typeArg) {
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
      throw new std::exception();
      break;
  }
}

void
Cpu::storeArg(const int& arg, const int& typeArg, int value) {
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
      break;
  }
}