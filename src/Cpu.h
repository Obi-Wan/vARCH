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

class Chipset; /* just a class declaration */

class Cpu {
public:
  Cpu(Chipset&, Mmu&);

  void init();

  void dumpRegistersAndMemory() const;

  int coreStep();
  
  const uint32_t & getTimeDelay() const throw() { return timeDelay; }

private:

  uint32_t timeDelay;

  /** The actual flags of the cpu */
  int flags;

  /** link to the chipset */
  Chipset& chipset;

  /** the MMU */
  Mmu& memoryController;

  /** The data registers */
  int regsData[NUM_REGS];

  /** The addresses registers */
  int regsAddr[NUM_REGS];

  struct StackPointers {
  private:
    uint32_t sSP; /* supervisor stack pointer */
    uint32_t uSP; /* user stack pointer */

    Cpu& cpu;
  public:
    StackPointers(Cpu& _c) : cpu(_c) { }
    
    void setStackPointer(const uint32_t& newSP) {
      if (cpu.flags & F_SVISOR) sSP = newSP; else uSP = newSP;
    }
    void setUStackPointer(const uint32_t& newSP) { uSP = newSP; }

    const uint32_t & getStackPointer() const throw() {
      return (cpu.flags & F_SVISOR) ? sSP : uSP;
    }
    const uint32_t & getUStackPointer() const throw() { return uSP; }

    void push(const int& data) {
      uint32_t& ref = (cpu.flags & F_SVISOR) ? sSP : uSP;
      cpu.memoryController.storeToMem(data,--ref);
    }
    int pop() {
      uint32_t& ref = (cpu.flags & F_SVISOR) ? sSP : uSP;
      return cpu.memoryController.loadFromMem(ref++);
    }

    void pushAllRegs() {
      uint32_t& ref = (cpu.flags & F_SVISOR) ? sSP : uSP;
      for(uint32_t i = 0; i < NUM_REGS; i++) {
        cpu.memoryController.storeToMem(cpu.regsData[i],--ref);
        cpu.memoryController.storeToMem(cpu.regsAddr[i],--ref);
      }
    }
    void popAllRegs() {
      uint32_t& ref = (cpu.flags & F_SVISOR) ? sSP : uSP;
      for(int i = NUM_REGS-1; i >= 0; i--) {
        cpu.regsAddr[i] = cpu.memoryController.loadFromMem(ref++);
        cpu.regsData[i] = cpu.memoryController.loadFromMem(ref++);
      }
    }
  } sP;

  /** The program counter */
  int progCounter;

  //|//////////////////////|//
  //|  Functions           |//
  //|//////////////////////|//
  int istructsOneArg(const int& istr, int& newFlags);
  int istructsZeroArg(const int& istr, int& newFlags);
  int istructsTwoArg(const int& istr, int& newFlags);
  int istructsThreeArg(const int& istr, int& newFlags);

  /* Arguments functions */
  int loadArg(const int& arg,const int& typeArg);
  void storeArg(const int& arg, const int& typeArg, int value);

  /* Regs functions */
  int getReg(const int& arg);
  void setReg(const int& arg, const int& value);

  void resetRegs() throw() {
    for( int i = 0; i < NUM_REGS; i++) regsData[i] = regsAddr[i] = 0;
  }

  /* Flags functions */
  void resetFlags(int& _flags) throw() {
    _flags -= _flags & ( F_ZERO + F_CARRY + F_NEGATIVE + F_OVERFLOW );
  }
  void restoreFlags(const int& _flags) throw() { flags = _flags; }
  int clearFlags(int mask) throw() {
    int oldFlags = flags;
    flags -= flags & mask;
    return oldFlags;
  }
  int setFlags(int mask) throw() {
    int oldFlags = flags;
    flags |= mask;
    return oldFlags;
  }
};

#endif	/* _CPU_H */

