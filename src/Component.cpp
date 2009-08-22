/* 
 * File:   Component.cpp
 * Author: ben
 * 
 * Created on 20 agosto 2009, 14.53
 */

#include "Component.h"

Component::Component() : simpleUnsafeResponse(0) { }

//Component::Component(const Component& orig) { }

Component::~Component() { }


void
Component::put(const int& signal) {
  if (signal == COMP_TYPE) simpleUnsafeResponse = COMP_CHAR;
}

int
Component::get() {
  return simpleUnsafeResponse;
}