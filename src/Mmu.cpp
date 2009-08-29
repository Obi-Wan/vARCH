/* 
 * File:   Mmu.cpp
 * Author: ben
 * 
 * Created on 19 agosto 2009, 15.37
 */

#include "Mmu.h"
#include <stdio.h>
#include <sstream>

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
Mmu::storeToMem(int data, int addr) throw(MmuException) {
  if ((addr >= limit) || (addr < 0)) {
    stringstream errorMess(string(""));
    errorMess << "StoreToMem Failed, wrong addr: " << addr
              << " upper limit: " << limit << "\n";
    throw MmuException(errorMess.str());
  }

  mainMem[addr + base] = data;
}

/** Loads data from memory
 *
 * @param addr The addres where to find data.
 *
 * @throws RuntimeException when address is wrong
 */
int
Mmu::loadFromMem(const int addr) const throw(MmuException) {
  if ((addr >= limit) || (addr < 0)) {
    stringstream errorMess(string(""));
    errorMess << "LoadFromMem Failed, wrong addr: " << addr
              << " upper limit: " << limit << "\n";
    throw MmuException(errorMess.str());
  }

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
Mmu::resetLimits(int base_new,int limit_new) throw(MmuException) {
  if ((base_new < 0) || (base_new > maxMem) ||
      (limit_new < 0) || (limit_new > maxMem) ||
      ((base_new + limit_new) > maxMem)) {
    stringstream errorMess(string(""));
    errorMess << "ReseLimits failed: Wrong Base or Limit. Base: " << base_new
              << " Limit: " << limit_new << " HardLimit: " << maxMem << "\n";
    throw MmuException(errorMess.str());
  }

  base = base_new;
  limit = limit_new;
}
