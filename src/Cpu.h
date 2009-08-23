/* 
 * File:   Cpu.h
 * Author: ben
 *
 * Created on 19 agosto 2009, 15.51
 */

#ifndef _CPU_H
#define	_CPU_H

#include "Mmu.h"
#include "Component.h"
#include "Chipset.h"
#include "../include/exceptions.h"

#define NUM_REGS 8

class Chipset; /* just a class declaration */

class Cpu : public Component {
public:
  Cpu(Chipset&, Mmu&);
  virtual ~Cpu();

  void init();

  void dumpRegistersAndMemory() const;

  int coreStep();
  
private:

  struct Flags {
    /** Flag that reports true if the last operation resulted in zero */
    bool zero;
    /** Flag that is true if the last result was negative */
    bool sign;
    /** Flag that is true if the last result overflawed */
    bool overflow;
  };

  /** link to the chipset */
  Chipset& chipset;

  /** the MMU */
  Mmu& memoryController;

  /** The data registers */
  int regsData[NUM_REGS];

//  /** The addresses registers */
//  int regsAddr[NUM_REGS];

  /** The program counter */
  int progCounter;

  /** The actual flags of the cpu */
  Flags flags;

  int istructsOneArg(const int& istr, Flags& newFlags)
            throw(WrongIstructionException);
  int istructsZeroArg(const int& istr, Flags& newFlags)
            throw(WrongIstructionException);
  int istructsTwoArg(const int& istr, Flags& newFlags)
            throw(WrongIstructionException);
  int istructsThreeArg(const int& istr, Flags& newFlags)
            throw(WrongIstructionException);

  int loadArg(const int& arg,const int& typeArg)
            throw(WrongArgumentException);
  void storeArg(const int& arg, const int& typeArg, int value)
            throw(WrongArgumentException);
  
  void resetFlags(Flags& _flags) {
    _flags.overflow = _flags.sign = _flags.zero = false;
  }
  void resetRegs() { for( int i = 0; i < NUM_REGS; i++) regsData[i] = 0; }
};

#endif	/* _CPU_H */

