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
#include "std_istructions.h"

union DoubleWord {
  uint8_t  u8[4];
  uint16_t u16[2];
  uint32_t u32;
};

class Mmu {
public:
  Mmu(const uint32_t & _maxMem, DoubleWord * _mem);
  Mmu(const Mmu& orig);

  const uint32_t &getMaxMem() const throw() { return maxMem; }
  const uint32_t &getLimit() const throw() { return limit; }

  /**
   * Stores data to memory
   *
   * @param data Contains data to write
   * @param addr Starting address of the data
   * @param size Size of the data (As coded in ScaleOfArgument)
   *
   * @return Time taken by the operation
   */
  uint32_t storeToMem(const DoubleWord & data, const uint32_t & addr,
      const uint8_t & size);
  /**
   * Stores data to memory
   *
   * @param data Contains data to write
   * @param addr Starting address of the data
   *
   * @return Time taken by the operation
   */
  uint32_t storeToMemUI32(const uint32_t& data, const uint32_t & addr) {
    DoubleWord _data; _data.u32 = data;
    return storeToMem(_data, addr, BYTE4);
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
  uint32_t loadFromMem(DoubleWord & data, const uint32_t & addr,
      const uint8_t & size) const;

  /** Sets Base and Limit to the desired values
   *
   * @param base_new The new base to set
   * @param limit_new The new limit to set
   *
   * @throws RuntimeException when one of the two is wrong
   */
  void resetLimits(const uint32_t & base_new, const uint32_t & limit_new);
  
  static const uint32_t accessTime = 100;

private:
  /** max lenth of istructions and data */
  uint32_t maxMem;

  /** the main memory */
  DoubleWord *mainMem;

  /** Base for program memory */
  uint32_t base;

  /** Limit for program memory */
  uint32_t limit;
};

#endif	/* _MMU_H */

