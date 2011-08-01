/* 
 * File:   Mmu.cpp
 * Author: ben
 * 
 * Created on 19 agosto 2009, 15.37
 */

#include "Mmu.h"

#include <sstream>

Mmu::Mmu(const uint32_t & _maxMem, int * _mem) : maxMem(_maxMem), mainMem(_mem)
{
  base = 0;
  limit = maxMem;
}

Mmu::Mmu(const Mmu& orig) : maxMem(orig.maxMem), mainMem(orig.mainMem) {
  base = orig.base;
  limit = orig.limit;
}



/** Stores data to memory
 *
 * @param data Data to store
 * @param addr The addres where to store data.
 *
 * @throws RuntimeException when address is wrong
 */
void
Mmu::storeToMem(const int32_t & data, const uint32_t & addr)
{
  if (addr >= limit) {
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
const int32_t &
Mmu::loadFromMem(const uint32_t & addr) const
{
  if (addr >= limit) {
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
Mmu::resetLimits(const uint32_t & base_new, const uint32_t & limit_new)
{
  if ( (base_new > maxMem) || (limit_new > maxMem)
       || ((base_new + limit_new) > maxMem))
  {
    stringstream errorMess(string(""));
    errorMess << "ReseLimits failed: Wrong Base or Limit. Base: " << base_new
              << " Limit: " << limit_new << " HardLimit: " << maxMem << "\n";
    throw MmuException(errorMess.str());
  }

  base = base_new;
  limit = limit_new;
}
