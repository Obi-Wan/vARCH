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

  virtual void put(const short int& request, const int& arg);
  int get();

  int isDataReady() { return dataReady; }
protected:
  int dataReady;

  int simpleUnsafeResponse;

private:
};

#endif	/* _COMPONENT_H */

