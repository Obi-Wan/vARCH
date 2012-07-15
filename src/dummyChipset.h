/* 
 * File:   dummyChipset.h
 * Author: ben
 *
 * Created on 3 marzo 2010, 14.34
 */

#ifndef _DUMMYCHIPSET_H
#define	_DUMMYCHIPSET_H

#include "Component.h"

class dummyChipset : public Component {
public:
  dummyChipset();
//  dummyChipset(const dummyChipset& orig);
//  virtual ~dummyChipset();
  
  void put(const short int& request, const int& arg);

  enum ChipsetActions {
    CHIP_INFO_BWORDS  = 0x00,
    CHIP_INFO_IN_MEM  = 0x01,
    DEVS_NUM          = 0x02,
    DEVS_LIST_IN_MEM  = 0x04,
  };
private:

};

#endif	/* _DUMMYCHIPSET_H */

