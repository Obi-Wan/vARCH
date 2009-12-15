/* 
 * File:   Mmu.h
 * Author: ben
 *
 * Created on 19 agosto 2009, 15.37
 */

#ifndef _MMU_H
#define	_MMU_H

#include "../include/exceptions.h"

class Mmu {
public:
  Mmu(const int _maxMem, int * _mem);
  Mmu(const Mmu& orig);

  int getMaxMem() { return maxMem; }
  
  void storeToMem(int data, int addr) throw(MmuException);
  int loadFromMem(const int addr) const throw(MmuException);
  void resetLimits(int base_new,int limit_new) throw(MmuException);
  
private:
  /** max lenth of istructions and data */
  int maxMem;

  /** the main memory */
  int *mainMem;

  /** Base for program memory */
  int base;

  /** Limit for program memory */
  int limit;
};

#endif	/* _MMU_H */

