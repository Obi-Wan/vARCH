/* 
 * File:   Cpu.cpp
 * Author: ben
 * 
 * Created on 19 agosto 2009, 15.51
 */

#include "Cpu.h"

#include "Chipset.h"
#include "std_istructions.h"
#include "asm_helpers.h"
#include "macros.h"

#ifdef DEBUG
# include "parser_definitions.h"
#endif

#include <cstdio>
#include <sstream>

#define SET_ARITM_FLAGS( x ) (( x < 0) ? F_NEGATIVE : ( (! x ) ? F_ZERO : 0 ))

Cpu::Cpu(Chipset& _chipset, Mmu& mC)
    : chipset(_chipset), memoryController(mC), sP(*this)
{
  init();
}

void
Cpu::init() {
  progCounter = 0;
  resetFlags(flags);
  flags += F_SVISOR | INT_PUT( INT_MAX_S_PR ); // start in supervisor mode
  resetRegs();
  sP.setStackPointer(memoryController.getLimit());
}

/** Writes to standard output the content of registers, program counter
 * and the content of the first region of memory
 */
void
Cpu::dumpRegistersAndMemory() const
{
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

  printf( "Stack pointers, user: %04u \tsupervisor: %04u\n",
          sP.getUStackPointer(), sP.getStackPointer());

  for(size_t i = 0; i < memoryController.getMaxMem(); i++) {
    printf( "Mem: %04lu Data: %12d\n", (uint64_t) i,
            memoryController.loadFromMem(i));
  }

//  restoreFlags(old);
}

int
Cpu::coreStep()
{
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
  timeDelay = 0;
  int newFlags = flags;
  resetFlags(newFlags);
  int currentInstr = memoryController.loadFromMem(progCounter++);

  DebugPrintf(("Istruzione nell'area di mem: %d num args: %d\n", progCounter-1,
         (currentInstr >> 30 ) & 3));

  int res = 0;

  try {
    switch ((currentInstr >> 30 ) & 3) {
      case 0:
        res = instructsZeroArg(currentInstr, newFlags);
        break;
      case 1:
        res = instructsOneArg(currentInstr, newFlags);
        break;
      case 2:
        res = instructsTwoArg(currentInstr, newFlags);
        break;
      case 3:
        res = instructsThreeArg(currentInstr, newFlags);
        break;
    }
  } catch (const WrongArgumentException & e) {
    printf("Wrong argument of the istruction. %s\n", e.what());
  } catch (const WrongInstructionException & e) {
    printf("Wrong istruction. %s\n", e.what());
  }
  flags = newFlags;
  return res;
}

inline int
Cpu::instructsZeroArg(const int& instr, int& newFlags)
{
  DebugPrintf(("  Instruction %s\n", ISet.getInstr(instr).c_str()));
  switch (instr) {
    case SLEEP:
      return instr;
    case REBOOT:
    case HALT:
      return (flags & F_SVISOR) ? instr : SLEEP;
      
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
      throw WrongInstructionException();
      break;
  }
  return 0;
}

inline int
Cpu::instructsOneArg(const int& instr, int& newFlags)
{
  const int typeArg = GET_ARG_1(instr);
  const int arg = memoryController.loadFromMem(progCounter++);
  DebugPrintf(("Type arg 1: 0x%X arg: 0x%X\n", typeArg, arg));
  int temp = loadArg(arg, typeArg);

  const int polishedInstr = instr - ARG_1(typeArg);

  DebugPrintf(("  Instruction %s\n", ISet.getInstr(polishedInstr).c_str()));
  switch (polishedInstr) {
    case NOT: {
      temp = ~temp;
      break;
    }
    case INCR: {
      int32_t old = temp;
      if (old > ++temp) flags += F_OVERFLOW;
      break;
    }
    case DECR: {
      int32_t old = temp;
      if (old < --temp) flags += F_OVERFLOW;
      break;
    }
    case COMP2: {
      temp = -temp;
      break;
    }
    case LSH: {
      temp <<= 1;
      break;
    }
    case RSH: {
      temp >>= 1;
      break;
    }

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
      throw WrongInstructionException();
      break;
  }

  if (polishedInstr < STACK || polishedInstr == POP || IS_PRE_POST_MOD(typeArg))
  {
    newFlags += SET_ARITM_FLAGS(temp);
    storeArg(arg, typeArg, temp);
  }

  return 0;
}

inline int
Cpu::instructsTwoArg(const int& instr, int& newFlags)
{
  const int typeArg1 = GET_ARG_1(instr);
  const int arg1 = memoryController.loadFromMem(progCounter++);
  DebugPrintf(("Type arg 1: 0x%X arg: 0x%X\n", typeArg1, arg1));
  int temp1 = loadArg(arg1, typeArg1);

  const int typeArg2 = GET_ARG_2(instr);
  const int arg2 = memoryController.loadFromMem(progCounter++);
  DebugPrintf(("Type arg 2: 0x%X arg: 0x%X\n", typeArg2, arg2));
  int temp2 = loadArg(arg2, typeArg2);

  const int polishedInstr = instr - ARG_1(typeArg1) - ARG_2(typeArg2);

  DebugPrintf(("  Instruction %s\n", ISet.getInstr(polishedInstr).c_str()));
  switch (polishedInstr) {
    case MOV:
      temp2 = temp1;
      break;
    case ADD: {
      int old = temp2;
      temp2 += temp1;
      if (old > temp2) flags += F_OVERFLOW;
      break;
    }
    case MULT: {
      int old = temp2;
      temp2 *= temp1;
      if (old > temp2) flags += F_OVERFLOW;
      break;
    }
    case SUB: {
      int old = temp2;
      temp2 -= temp1;
      if (old < temp2) flags += F_OVERFLOW;
      break;
    }
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
      throw WrongInstructionException();
      break;
  }

  if (polishedInstr <= XOR || polishedInstr == GET || IS_PRE_POST_MOD(typeArg2))
  {
    newFlags += SET_ARITM_FLAGS(temp2);
    storeArg(arg2, typeArg2, temp2); /* Operations that do modify args */
  }

  if (IS_PRE_POST_MOD(typeArg1)) { /* in case the first arg was modified */
    storeArg(arg1, typeArg1, temp1);
  }

  return 0;
}

inline int
Cpu::instructsThreeArg(const int& instr, int& newFlags)
{
  const int typeArg1 = GET_ARG_1(instr);
  const int arg1 = memoryController.loadFromMem(progCounter++);
  int temp1 = loadArg(arg1, typeArg1);

  const int typeArg2 = GET_ARG_2(instr);
  const int arg2 = memoryController.loadFromMem(progCounter++);
  int temp2 = loadArg(arg2, typeArg2);

  const int typeArg3 = GET_ARG_3(instr);
  const int arg3 = memoryController.loadFromMem(progCounter++);
  int temp3 = loadArg(arg3, typeArg3);

  const int polishedInstr = instr - ARG_1(typeArg1) - ARG_2(typeArg2)
                                  - ARG_3(typeArg3);

  DebugPrintf(("  Instruction %s\n", ISet.getInstr(polishedInstr).c_str()));
  switch (polishedInstr) {
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
      throw WrongInstructionException();
      break;
  }

  if (IS_PRE_POST_MOD(typeArg1)) { /* in case the first arg was modified */
    storeArg(arg1, typeArg1, temp1);
  }

  if (IS_PRE_POST_MOD(typeArg2)) { /* in case the second arg was modified */
    storeArg(arg2, typeArg2, temp2);
  }

  if (IS_PRE_POST_MOD(typeArg1)) { /* in case the third arg was modified */
    storeArg(arg3, typeArg3, temp3);
  }

  return 0;
}


inline int
Cpu::loadArg(const int& arg,const int& typeArg)
{
  const uint32_t relative = IS_RELATIVE(typeArg) * sP.getStackPointer();
  DebugPrintf(("  Relative: %d\n", relative));
  switch (typeArg & 0xf) {
    case CONST:
      DebugPrintf(("  CONST: %d\n", arg));
      return (arg + relative);

    case REG_PRE_INCR:
      DebugPrintf(("  REG_PRE_INCR: %d\n", arg));
      return (getReg(arg) +1);
    case REG_PRE_DECR:
      DebugPrintf(("  REG_PRE_DECR: %d\n", arg));
      return (getReg(arg) -1);
    case REG:
    case REG_POST_INCR:
    case REG_POST_DECR:
      DebugPrintf(("  REG / REG_POST_*: %d\n", arg));
      return getReg(arg);

    /// ADDR_P* modify the referenced content!
    case ADDR_PRE_INCR:
      DebugPrintf(("  ADDR_PRE_INCR: %d\n", arg));
      return (memoryController.loadFromMem(arg + relative) +1);
    case ADDR_PRE_DECR:
      DebugPrintf(("  ADDR_PRE_DECR: %d\n", arg));
      return (memoryController.loadFromMem(arg + relative) -1);
    case ADDR:
    case ADDR_POST_INCR:
    case ADDR_POST_DECR:
      DebugPrintf(("  ADDR / ADDR_POST_*: %d\n", arg));
      return memoryController.loadFromMem(arg + relative);

    /// ADDR_IN_REG_P* modify the register content!
    case ADDR_IN_REG_PRE_INCR:
      DebugPrintf(("  ADDR_IN_REG_PRE_INCR: %d\n", arg));
      return (memoryController.loadFromMem(getReg(arg) +1 + relative));
    case ADDR_IN_REG_PRE_DECR:
      DebugPrintf(("  ADDR_IN_REG_PRE_DECR: %d\n", arg));
      return (memoryController.loadFromMem(getReg(arg) -1 + relative));
    case ADDR_IN_REG:
    case ADDR_IN_REG_POST_INCR:
    case ADDR_IN_REG_POST_DECR:
      DebugPrintf(("  ADDR_IN_REG / ADDR_IN_REG_POST_*: %d\n", arg));
      return memoryController.loadFromMem(getReg(arg) + relative);
      
    default:
      throw WrongArgumentException("Failed in loading: wrong argument type");
  }
}

inline void
Cpu::storeArg(const int& arg, const int& typeArg, int value)
{
  const uint32_t relative = IS_RELATIVE(typeArg) * sP.getStackPointer();
  DebugPrintf(("  Relative: %d\n", relative));
  switch (typeArg & 0xf) {
    case REG:
    case REG_PRE_INCR:
    case REG_PRE_DECR:
      DebugPrintf(("  REG / REG_PRE_*: %d\n", arg));
      setReg(arg, value);
      break;
    case REG_POST_INCR:
      DebugPrintf(("  REG_POST_INCR: %d\n", arg));
      setReg(arg, ++value);
      break;
    case REG_POST_DECR:
      DebugPrintf(("  REG_POST_DECR: %d\n", arg));
      setReg(arg, --value);
      break;

    /// ADDR_P* modify the referenced content!
    case ADDR:
    case ADDR_PRE_INCR:
    case ADDR_PRE_DECR:
      DebugPrintf(("  ADDR / ADDR_PRE_*: %d\n", arg));
      memoryController.storeToMem(value, arg + relative);
      timeDelay += Mmu::accessTime;
      break;
    case ADDR_POST_INCR:
      DebugPrintf(("  ADDR_POST_INCR: %d\n", arg));
      memoryController.storeToMem(++value, arg + relative);
      timeDelay += Mmu::accessTime;
      break;
    case ADDR_POST_DECR:
      DebugPrintf(("  ADDR_POST_DECR: %d\n", arg));
      memoryController.storeToMem(--value, arg + relative);
      timeDelay += Mmu::accessTime;
      break;

    /// ADDR_IN_REG_P* modify the register content!
    case ADDR_IN_REG:
      DebugPrintf(("  ADDR_IN_REG: %d\n", arg));
      memoryController.storeToMem(value, getReg(arg) + relative);
      timeDelay += Mmu::accessTime;
      break;
    case ADDR_IN_REG_PRE_INCR:
    case ADDR_IN_REG_POST_INCR:
      DebugPrintf(("  ADDR_IN_REG_P*_INCR: %d\n", arg));
      memoryController.storeToMem(value, getReg(arg) + relative);
      setReg(arg, getReg(arg) +1);
      timeDelay += Mmu::accessTime;
      break;
    case ADDR_IN_REG_PRE_DECR:
    case ADDR_IN_REG_POST_DECR:
      DebugPrintf(("  ADDR_IN_REG_P*_DECR: %d\n", arg));
      memoryController.storeToMem(value, getReg(arg) + relative);
      setReg(arg, getReg(arg) -1);
      timeDelay += Mmu::accessTime;
      break;
    default:
      throw WrongArgumentException("Failed in storing: wrong argument type");
  }
}


inline int
Cpu::getReg(const int& arg)
{
  DebugPrintf( ("arg: %d type: %d, spec: %d\n", arg, arg / NUM_REGS,
                arg % NUM_REGS) );
  switch (arg) {
    case REG_DATA_1 ... REG_DATA_8:
      return regsData[ arg % NUM_REGS ];
    case REG_ADDR_1 ... REG_ADDR_8:
      return regsAddr[ arg % NUM_REGS ];
    case USER_STACK_POINTER:
      return (flags & F_SVISOR) ? sP.getUStackPointer() : sP.getStackPointer();
    case STACK_POINTER:
      return sP.getStackPointer();
    case STATE_REGISTER:
      return flags;
    case PROGRAM_COUNTER:
      return progCounter;
    default: {
      stringstream stream;
      stream << "No such register: " << arg << ". Couldn't load it.";
      throw WrongArgumentException(stream.str());
    }
  }
}

inline void
Cpu::setReg(const int& arg, const int& value)
{
  DebugPrintf( ("arg: %d type: %d, spec: %d\n", arg, arg / NUM_REGS,
                arg % NUM_REGS) );
  switch (arg) {
    case REG_DATA_1 ... REG_DATA_8:
      regsData[ arg % NUM_REGS ] = value;
      break;
    case REG_ADDR_1 ... REG_ADDR_8:
      regsAddr[ arg % NUM_REGS ] = value;
      break;
    case USER_STACK_POINTER:
      if (flags & F_SVISOR) {
        sP.setUStackPointer(value);
      } else {
        sP.setStackPointer(value);
      }
      break;
    case STACK_POINTER:
      sP.setStackPointer(value);
      break;
    case STATE_REGISTER:
      if (flags & F_SVISOR) {
        flags = value;
        break;
      } else {
        throw WrongArgumentException("Not Allowed to modify SR");
      }
    case PROGRAM_COUNTER:
      progCounter = value;
      break;
    default: {
      stringstream stream;
      stream << "No such register: " << arg << ". Couldn't set it.";
      throw WrongArgumentException(stream.str());
    }
  }
}
