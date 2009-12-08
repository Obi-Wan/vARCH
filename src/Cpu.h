/* 
 * File:   Cpu.h
 * Author: ben
 *
 * Created on 19 agosto 2009, 15.51
 */

#ifndef _CPU_H
#define	_CPU_H

#include "Mmu.h"
#include "../include/exceptions.h"

#define NUM_REGS 8

// Signals bits
#define F_CARRY       (1 << 0)
#define F_OVERFLOW    (1 << 1)
#define F_ZERO        (1 << 2)
#define F_NEGATIVE    (1 << 3)
#define F_EXTEND      (1 << 4)
#define F_INT_MASK    (1 << 5)
#define F_SVISOR      (1 << 6)
#define F_TRACE       (1 << 7)

// Interrupts priority conversion macros
#define INT_PUT(x)    (( x & 0xf ) << 8)
#define INT_GET(x)    (( x >> 8) & 0xf )
// Some useful priorities
#define INT_MAX_S_PR  0xf
#define INT_MIN_S_PR  0x8
#define INT_MAX_U_PR  0x7
#define INT_MIN_U_PR  0x0

class Chipset; /* just a class declaration */

class Cpu {
public:
  Cpu(Chipset&, Mmu&);
  virtual ~Cpu();

  void init();

  void dumpRegistersAndMemory() const;

  int coreStep();
  
private:

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
    int sSP; /* supervisor stack pointer */
    int uSP; /* user stack pointer */

    Cpu& cpu;
  public:
    StackPointers(Cpu& _c) : cpu(_c) { }
    
    void setStackPointer(const int& newSP) {
      if (cpu.flags & F_SVISOR) sSP = newSP; else uSP = newSP;
    }
    void setUStackPointer(const int& newSP) { uSP = newSP; }

    int getStackPointer() const { return (cpu.flags & F_SVISOR) ? sSP : uSP; }
    int getUStackPointer() const { return uSP; }

    void push(const int& data) {
      int& ref = (cpu.flags & F_SVISOR) ? sSP : uSP;
      cpu.memoryController.storeToMem(data,ref++);
    }
    int pop() {
      int& ref = (cpu.flags & F_SVISOR) ? sSP : uSP;
      return cpu.memoryController.loadFromMem(--ref);
    }

    void pushAllRegs() {
      int& ref = (cpu.flags & F_SVISOR) ? sSP : uSP;
      for(int i = 0; i < NUM_REGS; i++) {
        cpu.memoryController.storeToMem(cpu.regsData[i],ref++);
        cpu.memoryController.storeToMem(cpu.regsAddr[i],ref++);
      }
    }
    void popAllRegs() {
      int& ref = (cpu.flags & F_SVISOR) ? sSP : uSP;
      for(int i = NUM_REGS-1; i >= 0; i--) {
        cpu.regsAddr[i] = cpu.memoryController.loadFromMem(--ref);
        cpu.regsData[i] = cpu.memoryController.loadFromMem(--ref);
      }
    }
  } sP;

  /** The program counter */
  int progCounter;

  //|//////////////////////|//
  //|  Functions           |//
  //|//////////////////////|//
  int istructsOneArg(const int& istr, int& newFlags)
            throw(WrongIstructionException);
  int istructsZeroArg(const int& istr, int& newFlags)
            throw(WrongIstructionException);
  int istructsTwoArg(const int& istr, int& newFlags)
            throw(WrongIstructionException);
  int istructsThreeArg(const int& istr, int& newFlags)
            throw(WrongIstructionException);

  int loadArg(const int& arg,const int& typeArg)
            throw(WrongArgumentException);
  void storeArg(const int& arg, const int& typeArg, int value)
            throw(WrongArgumentException);

  void resetRegs() { for( int i = 0; i < NUM_REGS; i++) regsData[i] = regsAddr[i] = 0; }

  void resetFlags(int& _flags) {
    _flags -= _flags & ( F_ZERO + F_CARRY + F_NEGATIVE + F_OVERFLOW );
  }

  void restoreFlags(const int& _flags) { flags = _flags; }

  int clearFlags(int mask) {
    int oldFlags = flags;
    flags -= flags & mask;
    return oldFlags;
  }

  int setFlags(int mask) {
    int oldFlags = flags;
    flags |= mask;
    return oldFlags;
  }

  int getReg(const int& arg);
  void setReg(const int& arg, const int& value);
};

#endif	/* _CPU_H */

