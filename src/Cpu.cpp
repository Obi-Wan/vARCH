/* 
 * File:   Cpu.cpp
 * Author: ben
 * 
 * Created on 19 agosto 2009, 15.51
 */

#include "Cpu.h"

#include "Chipset.h"
#include "std_istructions.h"
#include "macros.h"

#ifdef DEBUG
# include "parser_definitions.h"
#endif

#include <cstdio>
#include <sstream>

inline
Cpu::ArgRecord::ArgRecord(const int32_t & packedType, const int32_t & data)
  : scale(GET_ARG_SCALE(packedType)), type(GET_ARG_TYPE(packedType))
  , raw_data(data)
{ }

inline
Cpu::StackPointers::StackPointers(Cpu& _c)
  : cpu(_c)
{
  uint32_t stackInitialPos = this->cpu.memoryController.getLimit();
  SCALE_ADDR_DECREM(stackInitialPos, BYTE4);
  uSP = sSP = stackInitialPos;
}

inline void
Cpu::StackPointers::push(const int32_t& data) {
  uint32_t& ref = (cpu.flags & F_SVISOR) ? sSP : uSP;
  SCALE_ADDR_DECREM(ref, BYTE4);
  cpu.memoryController.storeToMemUI32(uint32_t(data), ref);
}

inline int32_t
Cpu::StackPointers::pop() {
  uint32_t& ref = (cpu.flags & F_SVISOR) ? sSP : uSP;
  DoubleWord data;
  cpu.memoryController.loadFromMem(data, ref, BYTE4);
  SCALE_ADDR_INCREM(ref, BYTE4);
  return fromMemorySpace(data, BYTE4);
}

inline void
Cpu::StackPointers::pushAllRegs() {
  uint32_t& ref = (cpu.flags & F_SVISOR) ? sSP : uSP;
  for(uint32_t i = 0; i < NUM_REGS; i++) {
    SCALE_ADDR_DECREM(ref, BYTE4);
    cpu.memoryController.storeToMemUI32(uint32_t(cpu.regsData[i]), ref);
    SCALE_ADDR_DECREM(ref, BYTE4);
    cpu.memoryController.storeToMemUI32(uint32_t(cpu.regsAddr[i]), ref);
  }
}

inline void
Cpu::StackPointers::popAllRegs() {
  uint32_t& ref = (cpu.flags & F_SVISOR) ? sSP : uSP;
  DoubleWord data;
  for(int32_t i = NUM_REGS-1; i >= 0; i--) {
    cpu.memoryController.loadFromMem(data, ref, BYTE4);
    SCALE_ADDR_INCREM(ref, BYTE4);
    cpu.regsAddr[i] = fromMemorySpace(data, BYTE4);

    cpu.memoryController.loadFromMem(data, ref, BYTE4);
    SCALE_ADDR_INCREM(ref, BYTE4);
    cpu.regsData[i] = fromMemorySpace(data, BYTE4);
  }
}

#define SET_ARITM_FLAGS( x ) (( x < 0) ? F_NEGATIVE : ( (! x ) ? F_ZERO : 0 ))

Cpu::Cpu(Chipset& _chipset, Mmu& mC)
  : timeDelay(0), flags(0), chipset(_chipset), memoryController(mC), sP(*this)
  , progCounter(0)
{
  init();
}

void
Cpu::init() {
  progCounter = 0;
  resetFlags(flags);
  flags += F_SVISOR | INT_PUT( INT_MAX_S_PR ); // start in supervisor mode
  resetRegs();
  uint32_t stackInitialPos = memoryController.getLimit();
  SCALE_ADDR_DECREM(stackInitialPos, BYTE4);
  sP.setStackPointer(stackInitialPos);
}

/** Writes to standard output the content of registers, program counter
 * and the content of the first region of memory
 */
void
Cpu::dumpRegistersAndMemory() const
{
//  int old = setFlags(F_SVISOR);

  printf("Data Registers:");
  for(size_t i = 0; i < NUM_REGS; i++) {
    printf(" %d", regsData[i]);
  }
  
  printf("\nAddress Registers:");
  for (size_t i = 0; i < NUM_REGS; i++) {
    printf(" %d", regsAddr[i]);
  }

  printf("\nProgram Counter: %d\n",progCounter);

  printf( "Stack pointers, user: %04u \tsupervisor: %04u\n",
          sP.getUStackPointer(), sP.getStackPointer());

  DoubleWord doubleWord;
  for(uint32_t i = 0; i < memoryController.getMaxMem(); i += 4) {
    memoryController.loadFromMem(doubleWord, i, BYTE4);
    printf( "Mem: %04lu Data: %12d (31b %4d, %4d, %4d, %4d 0b)\n", (uint64_t) i,
            doubleWord.u32,
            doubleWord.u8[3], doubleWord.u8[2], doubleWord.u8[1], doubleWord.u8[0]);
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
      int32_t currentFlags = flags;
      flags += F_SVISOR;
      sP.push(currentFlags);
      sP.push(progCounter);

      DoubleWord doubleWord;

      timeDelay = memoryController.loadFromMem(doubleWord, regsAddr[7] +
              2 * intRecord.getDeviceId(), BYTE4);
      progCounter = doubleWord.u32;

      timeDelay += memoryController.loadFromMem( doubleWord,
              regsAddr[7] + 2 * intRecord.getDeviceId() + 1, BYTE4);
      restoreFlags( int32_t(doubleWord.u32) );

      chipset.topInterruptServed();
    }
  } else {
    timeDelay = 0;
  }

  //DebugPrintf(("Proceding with instructions execution\n"));
  // Now proced with instruction execution
  int32_t newFlags = flags;
  resetFlags(newFlags);
  DoubleWord fetchedInstr;
  timeDelay = memoryController.loadFromMem(fetchedInstr, progCounter, BYTE4);
  int32_t currentInstr = int32_t( fetchedInstr.u32 );

  SCALE_ADDR_INCREM(progCounter, BYTE4);

  int res = 0;

  try {
    switch (GET_NUM_ARGS(currentInstr)) {
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
  DebugPrintf(("Instruction %s (Mem pos: %04u, num args: %u)\n",
      ISet.getInstr(instr).c_str(), progCounter, GET_NUM_ARGS(instr) ));

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
  }
  return 0;
}

inline int32_t
Cpu::instructsOneArg(const int32_t& instr, int32_t& newFlags)
{
  const int32_t typeArg = GET_ARG_1(instr);
  const int32_t polishedInstr = instr - ARG_1(typeArg);
  DebugPrintf(("Instruction %s (Mem pos: %04u, num args: %u)\n",
      ISet.getInstr(polishedInstr).c_str(), progCounter, GET_NUM_ARGS(polishedInstr) ));

  const size_t scaleArg1 = (GET_ARG_TYPE(typeArg) == IMMED) ? GET_ARG_SCALE(typeArg) : BYTE4;

  DoubleWord rawArg;
  timeDelay += memoryController.loadFromMem(rawArg, progCounter, scaleArg1);
  SCALE_ADDR_INCREM(progCounter, scaleArg1);

  ArgRecord argRecord(typeArg, rawArg.u32);

  int32_t temp = 0;
  timeDelay += loadArg(temp, argRecord);

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
      if (flags & F_ZERO) progCounter += temp;
      break;
    case IFNJ:
      if (! (flags & F_ZERO)) progCounter += temp;
      break;
    case JMP:
      progCounter += temp;
      break;
    case JSR:
      sP.push(progCounter);
      progCounter += temp;
      break;
    case TCJ:
      if (flags & F_CARRY) progCounter += temp;
      break;
    case TZJ:
      if (flags & F_ZERO) progCounter += temp;
      break;
    case TOJ:
      if (flags & F_OVERFLOW) progCounter += temp;
      break;
    case TNJ:
      if (flags & F_NEGATIVE) progCounter += temp;
      break;
    case TSJ:
      if (flags & F_SVISOR) progCounter += temp;
      break;
      
    default:
      throw WrongInstructionException();
  }

  if ( polishedInstr < STACK || polishedInstr == POP
       || isAutoIncrDecrArg(argRecord) )
  {
    newFlags += SET_ARITM_FLAGS(temp);
    storeArg(temp, argRecord);
  }

  return 0;
}

inline int32_t
Cpu::instructsTwoArg(const int32_t& instr, int32_t& newFlags)
{
  const int32_t typeArg1 = GET_ARG_1(instr);
  const int32_t typeArg2 = GET_ARG_2(instr);
  const int32_t polishedInstr = instr - ARG_1(typeArg1) - ARG_2(typeArg2);
  DebugPrintf(("Instruction %s (Mem pos: %04u, num args: %u)\n",
      ISet.getInstr(polishedInstr).c_str(), progCounter, GET_NUM_ARGS(polishedInstr) ));

  const size_t scaleArg1 = (GET_ARG_TYPE(typeArg1) == IMMED) ? GET_ARG_SCALE(typeArg1) : BYTE4;
  const size_t scaleArg2 = (GET_ARG_TYPE(typeArg2) == IMMED) ? GET_ARG_SCALE(typeArg2) : BYTE4;

  DoubleWord rawArg1, rawArg2;
  timeDelay += memoryController.loadFromMem(rawArg1, progCounter, scaleArg1);
  SCALE_ADDR_INCREM(progCounter, scaleArg1);
  timeDelay += memoryController.loadFromMem(rawArg2, progCounter, scaleArg2);
  SCALE_ADDR_INCREM(progCounter, scaleArg2);

  ArgRecord argRecord1(typeArg1, rawArg1.u32);
  ArgRecord argRecord2(typeArg2, rawArg2.u32);

  int32_t temp1 = 0, temp2 = 0;
  timeDelay += loadArg(temp1, argRecord1);
  timeDelay += loadArg(temp2, argRecord2);

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
      DebugPrintf((" EQ: 0x%08X == 0x%08X\n", temp1, temp2));
      if (temp1 == temp2) newFlags += F_ZERO;
      break;
    case LO:
      DebugPrintf((" EQ: 0x%08X < 0x%08X\n", temp1, temp2));
      if (temp1 < temp2) newFlags += F_ZERO;
      break;
    case MO:
      DebugPrintf((" EQ: 0x%08X > 0x%08X\n", temp1, temp2));
      if (temp1 > temp2) newFlags += F_ZERO;
      break;
    case LE:
      DebugPrintf((" EQ: 0x%08X <= 0x%08X\n", temp1, temp2));
      if (temp1 <= temp2) newFlags += F_ZERO;
      break;
    case ME:
      DebugPrintf((" EQ: 0x%08X >= 0x%08X\n", temp1, temp2));
      if (temp1 >= temp2) newFlags += F_ZERO;
      break;
    case NEQ:
      DebugPrintf((" EQ: 0x%08X != 0x%08X\n", temp1, temp2));
      if (temp1 != temp2) newFlags += F_ZERO;
      break;
      
    default:
      throw WrongInstructionException();
      break;
  }

  if ( polishedInstr <= XOR || polishedInstr == GET
       || isAutoIncrDecrArg(argRecord2) )
  { /* Operations that do modify args */
    newFlags += SET_ARITM_FLAGS(temp2);
    storeArg(temp2, argRecord2);
  }

  if (isAutoIncrDecrArg(argRecord1)) { /* in case the first arg was modified */
    storeArg(temp1, argRecord1);
  }

  return 0;
}

inline int32_t
Cpu::instructsThreeArg(const int32_t& instr, int32_t& newFlags)
{
  const int32_t typeArg1 = GET_ARG_1(instr);
  const int32_t typeArg2 = GET_ARG_2(instr);
  const int32_t typeArg3 = GET_ARG_3(instr);
  const int32_t polishedInstr = instr - ARG_1(typeArg1) - ARG_2(typeArg2)
                                      - ARG_3(typeArg3);
  DebugPrintf(("Instruction %s (Mem pos: %04u, num args: %u)\n",
      ISet.getInstr(polishedInstr).c_str(), progCounter, GET_NUM_ARGS(polishedInstr) ));

  const size_t scaleArg1 = (GET_ARG_TYPE(typeArg1) == IMMED) ? GET_ARG_SCALE(typeArg1) : BYTE4;
  const size_t scaleArg2 = (GET_ARG_TYPE(typeArg2) == IMMED) ? GET_ARG_SCALE(typeArg2) : BYTE4;
  const size_t scaleArg3 = (GET_ARG_TYPE(typeArg3) == IMMED) ? GET_ARG_SCALE(typeArg3) : BYTE4;

  DoubleWord rawArg1, rawArg2, rawArg3;
  timeDelay += memoryController.loadFromMem(rawArg1, progCounter, scaleArg1);
  SCALE_ADDR_INCREM(progCounter, scaleArg1);
  timeDelay += memoryController.loadFromMem(rawArg2, progCounter, scaleArg2);
  SCALE_ADDR_INCREM(progCounter, scaleArg2);
  timeDelay += memoryController.loadFromMem(rawArg3, progCounter, scaleArg3);
  SCALE_ADDR_INCREM(progCounter, scaleArg3);

  ArgRecord argRecord1(typeArg1, rawArg1.u32);
  ArgRecord argRecord2(typeArg2, rawArg2.u32);
  ArgRecord argRecord3(typeArg3, rawArg3.u32);

  int32_t temp1 = 0, temp2 = 0, temp3 = 0;
  timeDelay += loadArg(temp1, argRecord1);
  timeDelay += loadArg(temp2, argRecord2);
  timeDelay += loadArg(temp3, argRecord3);

  switch (polishedInstr) {
    case BPUT:
      break;
    case BGET:
      break;
    case IFEQJ:
      if (temp2 == temp3) progCounter += temp1;
      break;
    case IFNEQJ:
      if (temp2 != temp3) progCounter += temp1;
      break;
    case IFLOJ:
      if (temp2 < temp3) progCounter += temp1;
      break;
    case IFMOJ:
      if (temp2 > temp3) progCounter += temp1;
      break;
    case IFLEJ:
      if (temp2 <= temp3) progCounter += temp1;
      break;
    case IFMEJ:
      if (temp2 >= temp3) progCounter += temp1;
      break;

    default:
      throw WrongInstructionException();
      break;
  }

  if (isAutoIncrDecrArg(argRecord1)) { /* in case the first arg was modified */
    storeArg(temp1, argRecord1);
  }

  if (isAutoIncrDecrArg(argRecord2)) { /* in case the second arg was modified */
    storeArg(temp2, argRecord2);
  }

  if (isAutoIncrDecrArg(argRecord3)) { /* in case the third arg was modified */
    storeArg(temp3, argRecord3);
  }

  return 0;
}


inline uint32_t
Cpu::loadArg(int32_t & temp, const ArgRecord & arg)
{
  DebugPrintf(("- LOAD:  Type: %9s, scale (Num bytes): %u, code: 0x%04X\n",
      ATypeSet.getItem(arg.type).c_str(), (1 << arg.scale), arg.raw_data));
  switch (arg.type) {
    case IMMED:
      DebugPrintf(("  CONST: %d\n", arg.raw_data));
      temp = int32_t(arg.raw_data);
      return 0;
    case REG: {
      const uint32_t reg_raw_data = GET_FIRST_REG(arg.raw_data);
      const uint32_t reg_pos = FILTER_PRE_POST(reg_raw_data);
      const int32_t pre = IS_PRE_POST_MOD(reg_raw_data)
                          * (!IS_POST(reg_raw_data))
                          * (1 - (2 * IS_DECR(reg_raw_data)) );
      temp = getReg(reg_pos) + pre;

      DebugPrintf(("  REG: %s, Mod: %s\n", RTypeSet.getItem(reg_pos).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(reg_raw_data)).c_str() ));
      return 0;
    }
    case DIRECT: {
      DoubleWord data;
      DebugPrintf(("  ADDR: %d\n", arg.raw_data));
      const uint32_t delay = memoryController.loadFromMem(data, arg.raw_data, arg.scale);
      temp = fromMemorySpace(data, arg.scale);
      return delay;
    }
    case REG_INDIR: {
      DoubleWord data;
      const uint32_t reg_raw_data = GET_FIRST_REG(arg.raw_data);
      const uint32_t reg_pos = FILTER_PRE_POST(reg_raw_data);
      const int32_t pre = IS_PRE_POST_MOD(reg_raw_data) * (1 << arg.scale)
                          * (!IS_POST(reg_raw_data))
                          * (1 - (2 * IS_DECR(reg_raw_data)) );
      const uint32_t addr = getReg(reg_pos) + pre;
      const uint32_t delay = memoryController.loadFromMem(data, addr, arg.scale);
      temp = fromMemorySpace(data, arg.scale);

      DebugPrintf(("  REG: %s, Mod: %s\n", RTypeSet.getItem(reg_pos).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(reg_raw_data)).c_str() ));

      return delay;
    }
    case MEM_INDIR: {
      DoubleWord data;
      const uint32_t reg_raw_data = GET_FIRST_REG(arg.raw_data);
      const uint32_t reg_pos = FILTER_PRE_POST(reg_raw_data);
      const int32_t pre = IS_PRE_POST_MOD(reg_raw_data) * (1 << arg.scale)
                          * (!IS_POST(reg_raw_data))
                          * (1 - (2 * IS_DECR(reg_raw_data)) );
      const uint32_t addr = getReg(reg_pos) + pre;
      uint32_t delay = memoryController.loadFromMem(data, addr, BYTE4);
      delay += memoryController.loadFromMem(data, data.u32, arg.scale);
      temp = fromMemorySpace(data, arg.scale);
#ifdef DEBUG
      DebugPrintf(("  REG: %s, Mod: %s\n", RTypeSet.getItem(reg_pos).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(reg_raw_data)).c_str() ));
#endif
      return delay;
    }
    case DISPLACED: {
      DoubleWord data;
      const int32_t displ = GET_BIG_DISPL(arg.raw_data);
      const uint32_t reg_raw_data = GET_FIRST_REG(arg.raw_data);
      const uint32_t reg_pos = FILTER_PRE_POST(reg_raw_data);
      const int32_t pre = IS_PRE_POST_MOD(reg_raw_data) * (1 << arg.scale)
                          * (!IS_POST(reg_raw_data))
                          * (1 - (2 * IS_DECR(reg_raw_data)) );
      const uint32_t addr = getReg(reg_pos) + pre + displ;
      const uint32_t delay = memoryController.loadFromMem(data, addr, arg.scale);
      temp = fromMemorySpace(data, arg.scale);

      DebugPrintf(("  REG: %s, Mod: %s\n", RTypeSet.getItem(reg_pos).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(reg_raw_data)).c_str() ));
      DebugPrintf(("  DISPL: %d, Final ADDR: %u\n", displ, addr ));
      return delay;
    }
    case INDEXED: {
      DoubleWord data;
      const uint32_t index_raw = GET_INDEX_REG(arg.raw_data);
      const uint32_t index = FILTER_PRE_POST(index_raw);
      const uint32_t reg_raw_data = GET_FIRST_REG(arg.raw_data);
      const uint32_t reg_pos = FILTER_PRE_POST(reg_raw_data);
      const int32_t pre = IS_PRE_POST_MOD(index_raw)
                          * (!IS_POST(index_raw))
                          * (1 - (2 * IS_DECR(index_raw)) );
      const uint32_t addr = getReg(reg_pos) + (pre + getReg(index)) * (1 << arg.scale);
      const uint32_t delay = memoryController.loadFromMem(data, addr, arg.scale);
      temp = fromMemorySpace(data, arg.scale);

      DebugPrintf(("  REG: %s, Mod: %s\n", RTypeSet.getItem(reg_pos).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(reg_raw_data)).c_str() ));
      DebugPrintf(("  INDEX: %s, Mod: %s\n", RTypeSet.getItem(index).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(index_raw)).c_str() ));
      return delay;
    }
    case INDX_DISP: {
      DoubleWord data;
      const int32_t displ = GET_INDEX_DISPL(arg.raw_data);
      const uint32_t index_raw = GET_INDEX_REG(arg.raw_data);
      const uint32_t index = FILTER_PRE_POST(index_raw);
      const uint32_t reg_raw_data = GET_FIRST_REG(arg.raw_data);
      const uint32_t reg_pos = FILTER_PRE_POST(reg_raw_data);
      const int32_t pre = IS_PRE_POST_MOD(index_raw)
                          * (!IS_POST(index_raw))
                          * (1 - (2 * IS_DECR(index_raw)) );
      const uint32_t addr = getReg(reg_pos) + (pre + getReg(index)) * (1 << arg.scale) + displ;
      const uint32_t delay = memoryController.loadFromMem(data, addr, arg.scale);
      temp = fromMemorySpace(data, arg.scale);

      DebugPrintf(("  REG: %s, Mod: %s\n", RTypeSet.getItem(reg_pos).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(reg_raw_data)).c_str() ));
      DebugPrintf(("  DISPL: %d, INDEX: %s, Mod: %s, Final ADDR: %u\n", displ,
                    RTypeSet.getItem(index).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(index_raw)).c_str(), addr ));
      return delay;
    }

    default:
      throw WrongArgumentException("Failed in loading: wrong argument type");
  }
}

inline uint32_t
Cpu::storeArg(const int32_t & temp, const ArgRecord & arg)
{
  DebugPrintf(("- STORE: Type: %9s, scale (Num bytes): %u, code: 0x%04X\n",
      ATypeSet.getItem(arg.type).c_str(), (1 << arg.scale), arg.raw_data));
  switch (arg.type) {
    case IMMED: {
      throw WrongArgumentException("Can't store an immediate!");
    }
    case REG: {
      const uint32_t reg_raw_data = GET_FIRST_REG(arg.raw_data);
      const uint32_t reg_pos = FILTER_PRE_POST(reg_raw_data);
      const int32_t post = IS_PRE_POST_MOD(arg.raw_data)
                           * (IS_POST(arg.raw_data))
                           * (1 - (2 * IS_DECR(arg.raw_data)) );
      setReg(reg_pos, temp + post);

      DebugPrintf(("  REG: %s, Mod: %s\n", RTypeSet.getItem(reg_pos).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(reg_raw_data)).c_str() ));
      return 0;
    }
    case DIRECT: {
      DoubleWord data;
      DebugPrintf(("  ADDR: %d\n", arg.raw_data));
      toMemorySpace(data, temp, arg.scale);
      const uint32_t delay = memoryController.storeToMem(data, arg.raw_data, arg.scale);
      return delay;
    }
    case REG_INDIR: {
      DoubleWord data;
      const uint32_t reg_raw_data = GET_FIRST_REG(arg.raw_data);
      const uint32_t reg_pos = FILTER_PRE_POST(reg_raw_data);
      const int32_t pre = IS_PRE_POST_MOD(arg.raw_data) * (1 << arg.scale)
                          * (!IS_POST(arg.raw_data))
                          * (1 - (2 * IS_DECR(arg.raw_data)) );
      const uint32_t addr = getReg(reg_pos) + pre;
      toMemorySpace(data, temp, arg.scale);
      const uint32_t delay = memoryController.storeToMem(data, addr, arg.scale);

      const int32_t post = IS_PRE_POST_MOD(arg.raw_data) * (1 << arg.scale)
                           * (IS_POST(arg.raw_data))
                           * (1 - (2 * IS_DECR(arg.raw_data)) );
      setReg(reg_pos, addr + post);

      DebugPrintf(("  REG: %s, Mod: %s\n", RTypeSet.getItem(reg_pos).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(reg_raw_data)).c_str() ));
      return delay;
    }
    case MEM_INDIR: {
      DoubleWord data;
      const uint32_t reg_raw_data = GET_FIRST_REG(arg.raw_data);
      const uint32_t reg_pos = FILTER_PRE_POST(reg_raw_data);
      const int32_t pre = IS_PRE_POST_MOD(arg.raw_data) * (1 << arg.scale)
                          * (!IS_POST(arg.raw_data))
                          * (1 - (2 * IS_DECR(arg.raw_data)) );
      const uint32_t addr = getReg(reg_pos) + pre;
      uint32_t delay = memoryController.loadFromMem(data, addr, BYTE4);
      const uint32_t new_addr = data.u32;
      toMemorySpace(data, temp, arg.scale);
      delay += memoryController.storeToMem(data, new_addr, arg.scale);

      const int32_t post = IS_PRE_POST_MOD(arg.raw_data) * (1 << arg.scale)
                           * (IS_POST(arg.raw_data))
                           * (1 - (2 * IS_DECR(arg.raw_data)) );
      setReg(reg_pos, addr + post);

      DebugPrintf(("  REG: %s, Mod: %s\n", RTypeSet.getItem(reg_pos).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(reg_raw_data)).c_str() ));
      return delay;
    }
    case DISPLACED: {
      DoubleWord data;
      const uint32_t reg_raw_data = GET_FIRST_REG(arg.raw_data);
      const uint32_t reg_pos = FILTER_PRE_POST(reg_raw_data);
      const int32_t displ = GET_BIG_DISPL(arg.raw_data);
      const int32_t pre = IS_PRE_POST_MOD(arg.raw_data) * (1 << arg.scale)
                          * (!IS_POST(arg.raw_data))
                          * (1 - (2 * IS_DECR(arg.raw_data)) );
      const uint32_t addr = getReg(reg_pos) + pre + displ;
      toMemorySpace(data, temp, arg.scale);
      const uint32_t delay = memoryController.storeToMem(data, addr, arg.scale);

      const int32_t post = IS_PRE_POST_MOD(arg.raw_data) * (1 << arg.scale)
                           * (IS_POST(arg.raw_data))
                           * (1 - (2 * IS_DECR(arg.raw_data)) );
      setReg(reg_pos, addr - displ + post);

      DebugPrintf(("  REG: %s, Mod: %s\n", RTypeSet.getItem(reg_pos).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(reg_raw_data)).c_str() ));
      DebugPrintf(("  DISPL: %d, Final ADDR: %u\n", displ, addr ));
      return delay;
    }
    case INDEXED: {
      DoubleWord data;
      const uint32_t index_raw = GET_INDEX_REG(arg.raw_data);
      const uint32_t index = FILTER_PRE_POST(index_raw);
      const uint32_t reg_raw_data = GET_FIRST_REG(arg.raw_data);
      const uint32_t reg_pos = FILTER_PRE_POST(reg_raw_data);
      const int32_t pre = IS_PRE_POST_MOD(index_raw)
                          * (!IS_POST(index_raw))
                          * (1 - (2 * IS_DECR(index_raw)) );
      const uint32_t addr = getReg(reg_pos) + (pre + getReg(index)) * (1 << arg.scale);
      toMemorySpace(data, temp, arg.scale);
      const uint32_t delay = memoryController.storeToMem(data, addr, arg.scale);

      const int32_t post = IS_PRE_POST_MOD(index_raw)
                           * (IS_POST(index_raw))
                           * (1 - (2 * IS_DECR(index_raw)) );
      setReg(index, pre + getReg(index) + post);

      DebugPrintf(("  REG: %s, Mod: %s\n", RTypeSet.getItem(reg_pos).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(reg_raw_data)).c_str() ));
      DebugPrintf(("  INDEX: %s, Mod: %s\n", RTypeSet.getItem(index).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(index_raw)).c_str() ));
      return delay;
    }
    case INDX_DISP: {
      DoubleWord data;
      const int32_t displ = GET_INDEX_DISPL(arg.raw_data);
      const uint32_t index_raw = GET_INDEX_REG(arg.raw_data);
      const uint32_t index = FILTER_PRE_POST(index_raw);
      const uint32_t reg_raw_data = GET_FIRST_REG(arg.raw_data);
      const uint32_t reg_pos = FILTER_PRE_POST(reg_raw_data);
      const int32_t pre = IS_PRE_POST_MOD(index_raw)
                          * (!IS_POST(index_raw))
                          * (1 - (2 * IS_DECR(index_raw)) );
      const uint32_t addr = getReg(reg_pos) + (pre + getReg(index)) * (1 << arg.scale) + displ;
      toMemorySpace(data, temp, arg.scale);
      const uint32_t delay = memoryController.storeToMem(data, addr, arg.scale);

      const int32_t post = IS_PRE_POST_MOD(index_raw)
                           * (IS_POST(index_raw))
                           * (1 - (2 * IS_DECR(index_raw)) );
      setReg(index, pre + getReg(index) + post);

      DebugPrintf(("  REG: %s, Mod: %s\n", RTypeSet.getItem(reg_pos).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(reg_raw_data)).c_str() ));
      DebugPrintf(("  DISPL: %d, INDEX: %s, Mod: %s, Final ADDR: %u\n", displ,
                    RTypeSet.getItem(index).c_str(),
                    MTypeSet.getItem(GET_ARG_MOD(index_raw)).c_str(), addr ));
      return delay;
    }

    default:
      throw WrongArgumentException("Failed in storing: wrong argument type");
  }
}


inline const int32_t
Cpu::getReg(const int32_t & regPos)
{
  switch (regPos) {
    case REG_DATA_0 ... REG_DATA_7:
      return regsData[ regPos % NUM_REGS ];
    case REG_ADDR_0 ... REG_ADDR_7:
      return regsAddr[ regPos % NUM_REGS ];
    case USER_STACK_POINTER:
      return (flags & F_SVISOR) ? sP.getUStackPointer() : sP.getStackPointer();
    case STACK_POINTER:
      return sP.getStackPointer();
    case STATE_REGISTER:
      return flags;
    case PROGRAM_COUNTER:
      return progCounter;
    default: {
      std::stringstream stream;
      stream << "No such register: " << regPos << ". Couldn't load it.";
      throw WrongArgumentException(stream.str());
    }
  }
}

inline void
Cpu::setReg(const int32_t & regPos, const int32_t & value)
{
  switch (regPos) {
    case REG_DATA_0 ... REG_DATA_7:
      regsData[ regPos % NUM_REGS ] = value;
      break;
    case REG_ADDR_0 ... REG_ADDR_7:
      regsAddr[ regPos % NUM_REGS ] = value;
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
      std::stringstream stream;
      stream << "No such register: " << regPos << ". Couldn't set it.";
      throw WrongArgumentException(stream.str());
    }
  }
}

inline int32_t
Cpu::fromMemorySpace(const DoubleWord & data, const uint8_t & scale)
{
  switch (scale) {
    case BYTE1:
      return static_cast<int32_t>(int8_t(data.u8[0])) & BWORD;
    case BYTE2:
      return static_cast<int32_t>(int16_t(data.u16[0])) & HWORD;
    case BYTE4:
      return int32_t(data.u32);
    default:
      throw WrongInstructionException("No support for 8 bytes data");
  }
}

inline void
Cpu::toMemorySpace(DoubleWord & data, const int32_t & value,
      const uint8_t & scale)
{
  switch (scale) {
    case BYTE1:
      data.u8[0] = uint8_t(static_cast<int8_t>(value));
      break;
    case BYTE2:
      data.u16[0] = uint16_t(static_cast<int16_t>(value));
      break;
    case BYTE4:
      data.u32 = uint32_t(value);
      break;
    default:
      throw WrongInstructionException("No support for 8 bytes data");
  }
}

inline bool
Cpu::isAutoIncrDecrArg(const ArgRecord & arg)
{
  switch (arg.type) {
    case REG:
    case REG_INDIR ... DISPLACED:
      return IS_PRE_POST_MOD(GET_FIRST_REG(arg.raw_data));
    case INDEXED ... INDX_DISP:
      return IS_PRE_POST_MOD(GET_INDEX_REG(arg.raw_data));
    default:
      return false;
  }
}

