/* 
 * File:   Mmu.h
 * Author: ben
 *
 * Created on 19 agosto 2009, 15.37
 */

#ifndef _MMU_H
#define	_MMU_H

#include "exceptions.h"
#include "macros.h"

class Mmu {
public:
  Mmu(const uint32_t & _maxMem, int32_t * _mem);
  Mmu(const Mmu& orig);

  const uint32_t &getMaxMem() const throw() { return maxMem; }
  
  void storeToMem(const int32_t & data, const uint32_t & addr);
  const int32_t &loadFromMem(const uint32_t & addr) const;
  void resetLimits(const uint32_t & base_new, const uint32_t & limit_new);
  
private:
  /** max lenth of istructions and data */
  uint32_t maxMem;

  /** the main memory */
  int32_t *mainMem;

  /** Base for program memory */
  uint32_t base;

  /** Limit for program memory */
  uint32_t limit;
};

#endif	/* _MMU_H */

