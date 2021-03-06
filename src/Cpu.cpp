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

INLINE
Cpu::ArgRecord::ArgRecord(const int32_t & packedType, const int32_t & data)
  : scale(GET_ARG_SCALE(packedType)), type(GET_ARG_TYPE(packedType))
  , raw_data(data)
{ }

INLINE void
Cpu::pushToStack(const int32_t & data) {
  uint32_t & ref = (flags & F_SVISOR) ? supStackPointer : usrStackPointer;
  SCALE_ADDR_DECREM(ref, BYTE4);
  this->memoryController.storeToMemUI32(uint32_t(data), ref);
}

INLINE int32_t
Cpu::popFromStack() {
  uint32_t & ref = (flags & F_SVISOR) ? supStackPointer : usrStackPointer;
  DoubleWord data;
  this->memoryController.loadFromMem(data, ref, BYTE4);
  SCALE_ADDR_INCREM(ref, BYTE4);
  return fromMemorySpace(data, BYTE4);
}

INLINE void
Cpu::pushAllRegsToStack() {
  uint32_t & ref = (flags & F_SVISOR) ? supStackPointer : usrStackPointer;
  for(uint32_t i = 0; i < NUM_REGS; i++) {
    SCALE_ADDR_DECREM(ref, BYTE4);
    this->memoryController.storeToMemUI32(uint32_t(this->regsData[i]), ref);
  }
}

INLINE void
Cpu::popAllRegsFromStack() {
  uint32_t & ref = (flags & F_SVISOR) ? supStackPointer : usrStackPointer;
  DoubleWord data;
  for(int32_t i = NUM_REGS-1; i >= 0; i--) {
    this->memoryController.loadFromMem(data, ref, BYTE4);
    SCALE_ADDR_INCREM(ref, BYTE4);
    this->regsData[i] = fromMemorySpace(data, BYTE4);
  }
}

#define SET_ARITM_FLAGS( x ) (( x < 0) ? F_NEGATIVE : ( (! x ) ? F_ZERO : 0 ))

Cpu::Cpu(Chipset& _chipset, Mmu& mC)
  : timeDelay(0), flags(0), chipset(_chipset), memoryController(mC)
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

  // Setting stack pointers
  uint32_t stackInitialPos = memoryController.getLimit();
  SCALE_ADDR_DECREM(stackInitialPos, BYTE4);
  usrStackPointer = supStackPointer = stackInitialPos;
}

/** Writes to standard output the content of registers, program counter
 * and the content of the first region of memory
 */
void
Cpu::dumpRegistersAndMemory() const
{
  printf(" Registers:\n");
  const uint32_t half_regs = NUM_REGS / 2 + NUM_REGS % 2;
  for(uint32_t curr_reg = 0; curr_reg < half_regs; curr_reg++) {
    printf("  R%02u: %08d (0x%08X)", curr_reg, regsData[curr_reg],
        static_cast<uint32_t>(regsData[curr_reg]));
    const uint32_t other_reg = half_regs + curr_reg;
    if (other_reg < NUM_REGS) {
      printf("  R%02u: %08d (0x%08X)", other_reg, regsData[other_reg],
          static_cast<uint32_t>(regsData[other_reg]));
    }
    printf("\n");
  }
  printf("\n");

  printf("     PC: 0x%08X (%04u)", progCounter, progCounter);
  printf("    sSP: 0x%08X (%04u)\n", supStackPointer, supStackPointer);
  printf("     FP: 0x%08X (%04u)", framePointer, framePointer);
  printf("    uSP: 0x%08X (%04u)\n", usrStackPointer, usrStackPointer);

  printf("\n");
  printf("         T S I E N Z O C\n");
  printf("     SR: %u %u %u %u %u %u %u %u\n", bool(F_TRACE & flags),
      bool(F_SVISOR & flags), bool(F_INT_MASK & flags), bool(F_EXTEND & flags),
      bool(F_NEGATIVE & flags), bool(F_ZERO & flags), bool(F_OVERFLOW & flags),
      bool(F_CARRY & flags));

  printf("\n");
  printf(" Memory:         Address    | Data\n");
  printf(" ---------------------------+------------------\n");
  const uint32_t min_addr = std::min(usrStackPointer, supStackPointer);
  const uint32_t max_addr = memoryController.getMaxMem() - (1 << BYTE4);
  DoubleWord doubleWord;
  for(uint32_t addr = max_addr; addr >= min_addr; SCALE_ADDR_DECREM(addr, BYTE4))
  {
    std::string ptr_descr = "";
    if (addr == framePointer) ptr_descr += "FP";
    if (addr == supStackPointer) {
      if (ptr_descr.empty()) ptr_descr += "sSP";
      else ptr_descr += ", sSP";
    }
    if (addr == usrStackPointer) {
      if (ptr_descr.empty()) ptr_descr += "uSP";
      else ptr_descr += ", uSP";
    }
    if (!ptr_descr.empty()) ptr_descr += " ->";
    memoryController.loadFromMem(doubleWord, addr, BYTE4);
    printf(" %15s 0x%08X | %04d (0x%08X) (31b %4d, %4d, %4d, %4d 0b)\n",
        ptr_descr.c_str(), addr, doubleWord.u32, doubleWord.u32,
        doubleWord.u8[3], doubleWord.u8[2], doubleWord.u8[1], doubleWord.u8[0]);
  }
  printf(" ---------------------------+------------------\n");
}

StdInstructions
Cpu::coreStep()
{
  //DebugPrintf(("Proceding with instructions execution\n"));
  // Now proced with instruction execution
  int32_t newFlags = flags;
  resetFlags(newFlags);
  DoubleWord fetchedInstr;
  timeDelay = memoryController.loadFromMem(fetchedInstr, progCounter, BYTE4);
  uint32_t currentInstr = fetchedInstr.u32;

  SCALE_ADDR_INCREM(progCounter, BYTE4);

  StdInstructions res = SLEEP;

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

INLINE StdInstructions
Cpu::instructsZeroArg(const uint32_t & instr, int32_t & newFlags)
{
  DebugPrintf(("Instruction %s (Mem pos: %04u, num args: %u)\n",
      ISet.getInstr(instr).c_str(), progCounter, GET_NUM_ARGS(instr) ));

  switch (instr) {
    case SLEEP:
      return static_cast<StdInstructions>(instr);
    case REBOOT:
    case HALT:
      return (flags & F_SVISOR) ? static_cast<StdInstructions>(instr) : SLEEP;
      
    case PUSHA:
      pushAllRegsToStack();
      break;
    case POPA:
      popAllRegsFromStack();
      break;
    case RET:
      progCounter = popFromStack();
      break;
    case RETEX:
      flags += F_SVISOR;
      progCounter = popFromStack();
      newFlags = popFromStack();
      break;
    case DUMP:
      this->dumpRegistersAndMemory();
      break;

    default:
      throw WrongInstructionException();
  }
  return SLEEP;
}

INLINE StdInstructions
Cpu::instructsOneArg(const uint32_t & instr, int32_t& newFlags)
{
  const uint32_t typeArg = GET_ARG_1(instr);
  const uint32_t polishedInstr = instr - ARG_1(typeArg);

  DebugPrintf(("Instruction %s (Mem pos: %04u, num args: %u)\n",
      ISet.getInstr(polishedInstr).c_str(), progCounter, GET_NUM_ARGS(polishedInstr) ));

  const uint8_t scaleArg1 = (GET_ARG_TYPE(typeArg) == IMMED) ? GET_ARG_SCALE(typeArg) : BYTE4;

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
      setStackPointer(temp);
      break;
    case PUSH:
      pushToStack(temp);
      break;
    case POP:
      temp = popFromStack();
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
      pushToStack(progCounter);
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

  return SLEEP;
}

INLINE StdInstructions
Cpu::instructsTwoArg(const uint32_t & instr, int32_t & newFlags)
{
  const uint32_t typeArg1 = GET_ARG_1(instr);
  const uint32_t typeArg2 = GET_ARG_2(instr);
  const uint32_t polishedInstr = instr - ARG_1(typeArg1) - ARG_2(typeArg2);
  DebugPrintf(("Instruction %s (Mem pos: %04u, num args: %u)\n",
      ISet.getInstr(polishedInstr).c_str(), progCounter, GET_NUM_ARGS(polishedInstr) ));

  const uint8_t scaleArg1 = (GET_ARG_TYPE(typeArg1) == IMMED) ? GET_ARG_SCALE(typeArg1) : BYTE4;
  const uint8_t scaleArg2 = (GET_ARG_TYPE(typeArg2) == IMMED) ? GET_ARG_SCALE(typeArg2) : BYTE4;

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

  return SLEEP;
}

INLINE StdInstructions
Cpu::instructsThreeArg(const uint32_t & instr, int32_t & newFlags)
{
  const uint32_t typeArg1 = GET_ARG_1(instr);
  const uint32_t typeArg2 = GET_ARG_2(instr);
  const uint32_t typeArg3 = GET_ARG_3(instr);
  const uint32_t polishedInstr = instr - ARG_1(typeArg1) - ARG_2(typeArg2) - ARG_3(typeArg3);

  DebugPrintf(("Instruction %s (Mem pos: %04u, num args: %u)\n",
      ISet.getInstr(polishedInstr).c_str(), progCounter, GET_NUM_ARGS(polishedInstr) ));

  const uint8_t scaleArg1 = (GET_ARG_TYPE(typeArg1) == IMMED) ? GET_ARG_SCALE(typeArg1) : BYTE4;
  const uint8_t scaleArg2 = (GET_ARG_TYPE(typeArg2) == IMMED) ? GET_ARG_SCALE(typeArg2) : BYTE4;
  const uint8_t scaleArg3 = (GET_ARG_TYPE(typeArg3) == IMMED) ? GET_ARG_SCALE(typeArg3) : BYTE4;

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

  return SLEEP;
}


INLINE uint32_t
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

INLINE uint32_t
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


INLINE const int32_t
Cpu::getReg(const int32_t & regPos)
{
  switch (regPos) {
    case REG_DATA_0 ... (NUM_REGS-1):
      return regsData[regPos];
    case USER_STACK_POINTER:
      return getUStackPointer();
    case STACK_POINTER:
      return getStackPointer();
    case FRAME_POINTER:
      return framePointer;
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

INLINE void
Cpu::setReg(const int32_t & regPos, const int32_t & value)
{
  switch (regPos) {
    case REG_DATA_0 ... (NUM_REGS-1):
      regsData[regPos] = value;
      break;
    case USER_STACK_POINTER:
      setUStackPointer(value);
      break;
    case STACK_POINTER:
      setStackPointer(value);
      break;
    case FRAME_POINTER:
      framePointer = value;
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

INLINE int32_t
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

INLINE void
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

INLINE bool
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

