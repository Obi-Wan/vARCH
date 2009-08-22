/* 
 * File:   Mmu.cpp
 * Author: ben
 * 
 * Created on 19 agosto 2009, 15.37
 */

#include "Mmu.h"
#include <exception>
#include <stdio.h>

Mmu::Mmu(const int _maxMem, int * _mem) : maxMem(_maxMem), mainMem(_mem) {
  base = 0;
  limit = maxMem;
}

Mmu::Mmu(const Mmu& orig) : maxMem(orig.maxMem), mainMem(orig.mainMem) {
  base = orig.base;
  limit = orig.limit;
}

Mmu::~Mmu() { }



/** Stores data to memory
 *
 * @param data Data to store
 * @param addr The addres where to store data.
 *
 * @throws RuntimeException when address is wrong
 */
void
Mmu::storeToMem(int data, int addr) {
  if ((addr >= limit) || (addr < 0))
        throw new std::exception();//("StoreToMem Failed: wrong addr!");

  mainMem[addr + base] = data;
}

/** Loads data from memory
 *
 * @param addr The addres where to find data.
 *
 * @throws RuntimeException when address is wrong
 */
int
Mmu::loadFromMem(const int addr) const {
  if ((addr >= limit) || (addr < 0))
        throw new std::exception();//("LoadFromMem Failed: wrong addr!");

  return mainMem[addr + base];
}

/** Sets Base and Limit to the desired values
 *
 * @param base_new The new base to set
 * @param limit_new The new limit to set
 *
 * @throws RuntimeException when one of the two is wrong
 */
void
Mmu::resetLimits(int base_new,int limit_new) {
  if ((base_new < 0) || (base_new > maxMem) ||
      (limit_new < 0) || (limit_new > maxMem) ||
      ((base_new + limit_new) > maxMem))
        throw new std::exception();//("ReseLimits failed: Wrong Base or Limit!");

  base = base_new;
  limit = limit_new;
}
