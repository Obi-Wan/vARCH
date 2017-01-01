/* 
 * File:   Cpu.h
 * Author: ben
 *
 * Created on 19 agosto 2009, 15.51
 */

#ifndef _CPU_H
#define	_CPU_H

#include "Mmu.h"
#include "exceptions.h"
#include "CpuDefinitions.h"
#include "std_istructions.h"

class Chipset; /* just a class declaration */

class Cpu {
public:
  Cpu(Chipset &, Mmu &);

  void init();

  void dumpRegistersAndMemory() const;

  StdInstructions coreStep();
  
  const uint32_t & getTimeDelay() const throw() { return timeDelay; }

private:

  uint32_t timeDelay;

  /** The actual flags of the cpu */
  int32_t flags;

  /** link to the chipset */
  Chipset& chipset;

  /** the MMU */
  Mmu& memoryController;

  /** The raw_data registers */
  int32_t regsData[NUM_REGS];

  /** The program counter */
  uint32_t progCounter;

  /** The frame pointer
   * It is a dedicated register, so that we don't need to "waste" a normal
   * register for this */
  uint32_t framePointer;

  /** supervisor stack pointer */
  uint32_t supStackPointer;
  /** user stack pointer */
  uint32_t usrStackPointer;

  void setStackPointer(const uint32_t & newSP) {
    if (flags & F_SVISOR) supStackPointer = newSP; else usrStackPointer = newSP;
  }
  void setUStackPointer(const uint32_t & newSP) { usrStackPointer = newSP; }

  const uint32_t & getStackPointer() const throw() {
    return (flags & F_SVISOR) ? supStackPointer : usrStackPointer;
  }
  const uint32_t & getUStackPointer() const throw() { return usrStackPointer; }

  void pushToStack(const int32_t & data);
  int32_t popFromStack();

  void pushAllRegsToStack();
  void popAllRegsFromStack();

  //|//////////////////////|//
  //|  Functions           |//
  //|//////////////////////|//
  StdInstructions instructsZeroArg(const uint32_t & instr, int32_t & newFlags);
  StdInstructions instructsOneArg(const uint32_t & instr, int32_t & newFlags);
  StdInstructions instructsTwoArg(const uint32_t & instr, int32_t & newFlags);
  StdInstructions instructsThreeArg(const uint32_t & instr, int32_t & newFlags);

  /**
   * Temporary record for arguments processing
   */
  struct ArgRecord {
    uint8_t scale;
    uint8_t type;
    uint32_t raw_data;

    ArgRecord(const int32_t & packedType, const int32_t & data);
  };

  /* Arguments functions */
  uint32_t loadArg(int32_t & temp, const ArgRecord &argRecord);
  uint32_t storeArg(const int32_t & value, const ArgRecord & argRecord);

  /* Regs functions */
  const int32_t getReg(const int32_t & arg);
  void setReg(const int32_t & arg, const int32_t & value);

  void resetRegs() throw() {
    for(size_t i = 0; i < NUM_REGS; i++) { regsData[i] = 0; }
  }

  /* Flags functions */
  void resetFlags(int32_t& _flags) throw() {
    _flags -= _flags & ( F_ZERO + F_CARRY + F_NEGATIVE + F_OVERFLOW );
  }
  void restoreFlags(const int32_t & _flags) throw() { flags = _flags; }
  int32_t clearFlags(const int32_t & mask) throw() {
    int32_t oldFlags = flags;
    flags -= flags & mask;
    return oldFlags;
  }
  int32_t setFlags(const int32_t & mask) throw() {
    int32_t oldFlags = flags;
    flags |= mask;
    return oldFlags;
  }

  static int32_t fromMemorySpace(const DoubleWord & data, const uint8_t & scale);
  static void toMemorySpace(DoubleWord & data, const int32_t & value,
      const uint8_t & scale);

  static bool isAutoIncrDecrArg(const ArgRecord & arg);
};

#endif	/* _CPU_H */

