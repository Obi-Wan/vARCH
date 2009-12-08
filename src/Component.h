/* 
 * File:   Component.h
 * Author: ben
 *
 * Created on 20 agosto 2009, 14.53
 */

#ifndef _COMPONENT_H
#define	_COMPONENT_H

#include "InterruptDevice.h"

#define DATA_READY_TRUE     1
#define DATA_READY_FALSE    0
#define DATA_READY_ERROR   -1

#define REQUEST_TYPE_MASK   0x000000ff
#define REQUEST_ARG_MASK    0x00ffffff

#define REQUEST_SHIFT(x)    (x >> 23)

class Component : public InterruptDevice {
public:
  Component();
//  Component(const Component& orig);
  virtual ~Component();

  enum ComponentType {
    COMP_CHAR     =     (1 <<  0),
    COMP_BLOCK    =     (1 <<  1),
    COMP_KEYBOARD =     (1 <<  2),
    COMP_TIMER    =     (1 <<  3),
    COMP_CONSOLE  =     (1 <<  4),
  };

  enum ComponentRequest {
    COMP_TYPE,
    COMP_GET_FEATURES,
    COMP_SET_FEATURES,
  };

  virtual void put(const int& signal);
  int get();

  int isDataReady() { return dataReady; }
protected:
  int dataReady;

  int simpleUnsafeResponse;

private:
};

#endif	/* _COMPONENT_H */

