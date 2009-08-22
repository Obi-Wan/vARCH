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

class Cpu : public Component {
public:
  Cpu(Mmu & mC);
  virtual ~Cpu();

  void dumpRegistersAndMemory();

  int coreStep();
  
private:

  /** the MMU */
  Mmu &memoryController;

  /** The data registers */
  int regsData[8];

//  /** The addresses registers */
//  int regsAddr[8];

  /** The program counter */
  int progCounter;

  /** Flag that reports true if the last operation resulted in zero */
  bool zero;

  /** Flag that is true if the last result was negative */
  bool sign;

  /** Flag that is true if the last result overflawed */
  bool overflow;

  int istructsOneArg(const int& istr);
  int istructsZeroArg(const int& istr);
  int istructsTwoArg(const int& istr);
  int istructsThreeArg(const int& istr);

  int loadArg(const int& arg,const int& typeArg);
  void storeArg(const int& arg, const int& typeArg, int value);
};

#endif	/* _CPU_H */

