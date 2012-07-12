/* 
 * File:   Mmu.cpp
 * Author: ben
 * 
 * Created on 19 agosto 2009, 15.37
 */

#include "Mmu.h"

#include <sstream>

Mmu::Mmu(const uint32_t & _maxMem, DoubleWord * _mem)
  : maxMem(_maxMem), mainMem(_mem)
{
  base = 0;
  limit = maxMem;
}

Mmu::Mmu(const Mmu& orig) : maxMem(orig.maxMem), mainMem(orig.mainMem) {
  base = orig.base;
  limit = orig.limit;
}

/**
 * Stores data to memory
 *
 * @param data Contains data to write
 * @param addr Starting address of the data
 * @param size Size of the data (As coded in ScaleOfArgument)
 *
 * @return Time taken by the operation
 */
uint32_t
Mmu::storeToMem(const DoubleWord & data, const uint32_t & addr,
    const uint8_t & size)
{
  if ((addr + (1 << size)) >= limit) {
    stringstream errorMess(string(""));
    errorMess << "LoadFromMem Failed, wrong addr: " << addr
              << " size: " << (1 << size)
              << " upper limit: " << limit << "\n";
    throw MmuException(errorMess.str());
  }

  const uint32_t blockNum = ((addr + base) >> 2);
  const uint32_t blockOffset = ((addr + base) & 3);

  switch (size) {
    case BYTE1: {
      /* Aligned access */
      mainMem[blockNum].u8[blockOffset] = data.u8[0];
      return Mmu::accessTime;
    }
    case BYTE2: {
      switch (blockOffset) {
        case 0:
          /* Aligned access */
          mainMem[blockNum].u16[0] = data.u16[0];
          return Mmu::accessTime;
        case 2:
          /* Aligned access */
          mainMem[blockNum].u16[1] = data.u16[0];
          return Mmu::accessTime;
        case 1:
          /* Misaligned access */
          mainMem[blockNum].u8[1] = data.u8[0];
          mainMem[blockNum].u8[2] = data.u8[1];
          return Mmu::accessTime * 2;
        case 3:
          /* Misaligned access */
          mainMem[blockNum].u8[3] = data.u8[0];
          mainMem[blockNum+1].u8[0] = data.u8[1];
          return Mmu::accessTime * 2;
        default:
          break;
      }
      throw MmuException("We shouldn't get here");
    }
    case BYTE4: {
      switch (blockOffset) {
        case 0:
          /* Aligned access */
          mainMem[blockNum].u32 = data.u32;
          return Mmu::accessTime;
        case 1:
          /* Misaligned access */
          mainMem[blockNum].u8[1] = data.u8[0];
          mainMem[blockNum].u8[2] = data.u8[1];
          mainMem[blockNum].u8[3] = data.u8[2];
          mainMem[blockNum+1].u8[0] = data.u8[3];
          return Mmu::accessTime * 2;
        case 2:
          /* Misaligned access */
          mainMem[blockNum].u16[1] = data.u16[0];
          mainMem[blockNum+1].u16[0] = data.u16[1];
          return Mmu::accessTime * 2;
        case 3:
          /* Misaligned access */
          mainMem[blockNum].u8[3] = data.u8[0];
          mainMem[blockNum+1].u8[0] = data.u8[1];
          mainMem[blockNum+1].u8[1] = data.u8[2];
          mainMem[blockNum+1].u8[2] = data.u8[3];
          return Mmu::accessTime * 2;
        default:
          break;
      }
      throw MmuException("We shouldn't get here");
    }
    default:
      stringstream errorMess(string(""));
      errorMess << "Size of " << (1 << size)
                << " bytes not supported (just 1, 2, 4)\n";
      throw MmuException(errorMess.str());
  }
}

/**
 * Loads data from memory
 *
 * @param data Data to read
 * @param addr Starting address of the data
 * @param size Size of the data (As coded in ScaleOfArgument)
 *
 * @return Time taken by the operation
 */
uint32_t
Mmu::loadFromMem(DoubleWord & data, const uint32_t & addr, const uint8_t & size)
  const
{
  if ((addr + (1 << size) ) > limit) {
    stringstream errorMess(string(""));
    errorMess << "LoadFromMem Failed, wrong addr: " << addr
              << ", size: " << (1 << size)
              << ", upper limit: " << limit << "\n";
    throw MmuException(errorMess.str());
  }

  const uint32_t blockNum = ((addr + base) >> 2);
  const uint32_t blockOffset = ((addr + base) & 3);

  data.u32 = 0;
  switch (size) {
    case BYTE1: {
      /* Aligned access */
      data.u8[0] = mainMem[blockNum].u8[blockOffset];
      return Mmu::accessTime;
    }
    case BYTE2: {
      switch (blockOffset) {
        case 0:
          /* Aligned access */
          data.u16[0] = mainMem[blockNum].u16[0];
          return Mmu::accessTime;
        case 2:
          /* Aligned access */
          data.u16[0] = mainMem[blockNum].u16[1];
          return Mmu::accessTime;
        case 1:
          /* Misaligned access */
          data.u8[0] = mainMem[blockNum].u8[1];
          data.u8[1] = mainMem[blockNum].u8[2];
          return Mmu::accessTime * 2;
        case 3:
          /* Misaligned access */
          data.u8[0] = mainMem[blockNum].u8[3];
          data.u8[1] = mainMem[blockNum+1].u8[0];
          return Mmu::accessTime * 2;
        default:
          break;
      }
      throw MmuException("We shouldn't get here");
    }
    case BYTE4: {
      switch (blockOffset) {
        case 0:
          /* Aligned access */
          data.u32 = mainMem[blockNum].u32;
          return Mmu::accessTime;
        case 1:
          /* Misaligned access */
          data.u8[0] = mainMem[blockNum].u8[1];
          data.u8[1] = mainMem[blockNum].u8[2];
          data.u8[2] = mainMem[blockNum].u8[3];
          data.u8[3] = mainMem[blockNum+1].u8[0];
          return Mmu::accessTime * 2;
        case 2:
          /* Misaligned access */
          data.u16[0] = mainMem[blockNum].u16[1];
          data.u16[1] = mainMem[blockNum+1].u16[0];
          return Mmu::accessTime * 2;
        case 3:
          /* Misaligned access */
          data.u8[0] = mainMem[blockNum].u8[3];
          data.u8[1] = mainMem[blockNum+1].u8[0];
          data.u8[2] = mainMem[blockNum+1].u8[1];
          data.u8[3] = mainMem[blockNum+1].u8[2];
          return Mmu::accessTime * 2;
        default:
          break;
      }
      throw MmuException("We shouldn't get here");
    }
    default:
      stringstream errorMess(string(""));
      errorMess << "Size of " << (1 << size)
                << " bytes not supported (just 1, 2, 4)\n";
      throw MmuException(errorMess.str());
  }
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
