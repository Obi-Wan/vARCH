/* 
 * File:   Mmu.h
 * Author: ben
 *
 * Created on 19 agosto 2009, 15.37
 */

#ifndef _MMU_H
#define	_MMU_H

class Mmu {
public:
  Mmu(const int _maxMem, int * _mem);
  Mmu(const Mmu& orig);
  virtual ~Mmu();

  int getMaxMem() { return maxMem; }
  
  void storeToMem(int data, int addr);
  int loadFromMem(const int addr) const;
  void resetLimits(int base_new,int limit_new);
  
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

