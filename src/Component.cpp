/* 
 * File:   Component.cpp
 * Author: ben
 * 
 * Created on 20 agosto 2009, 14.53
 */

#include "Component.h"

Component::Component() : simpleUnsafeResponse(0), dataReady(DATA_READY_FALSE) { }

//Component::Component(const Component& orig) { }

Component::~Component() { }


void
Component::put(const int& signal) {
  switch (signal) {
    case COMP_TYPE:
      simpleUnsafeResponse = COMP_CHAR;
      dataReady = DATA_READY_TRUE;
      break;
    case COMP_GET_FEATURES:
      dataReady = DATA_READY_ERROR;
      break;
  }
}

int
Component::get() {
  dataReady = DATA_READY_FALSE;
  return simpleUnsafeResponse;
}