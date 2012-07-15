/* 
 * File:   dummyChipset.cpp
 * Author: ben
 * 
 * Created on 3 marzo 2010, 14.34
 */

#include "dummyChipset.h"

dummyChipset::dummyChipset() { }
//
//dummyChipset::dummyChipset(const dummyChipset& orig) { }
//
//dummyChipset::~dummyChipset() { }

void
dummyChipset::put(const short int& request, const int& arg) {
  switch (request) {
    case CHIP_INFO_BWORDS:
      /* How many Single WORDS chip information takes */
      break;
    case CHIP_INFO_IN_MEM:
      /* Let's put chip information in the memory area specified by arg */
      break;
    case DEVS_NUM:
    case DEVS_LIST_IN_MEM:
      break;
    default:
      break;
  }
}

