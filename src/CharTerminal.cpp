/* 
 * File:   CharTerminal.cpp
 * Author: ben
 * 
 * Created on 8 dicembre 2009, 21.13
 */

#include <stdio.h>

#include "CharTerminal.h"

CharTerminal::CharTerminal() { }

//CharTerminal::CharTerminal(const CharTerminal& orig) { }
//
//CharTerminal::~CharTerminal() { }


void
CharTerminal::put(const int& signal) {
  switch (signal & REQUEST_TYPE_MASK) {
    case COMP_TYPE:
      simpleUnsafeResponse = COMP_CONSOLE;
      dataReady = DATA_READY_TRUE;
      break;
    case COMP_SET_FEATURES:
      printf("%c",signal & CHAR_MASK);
      simpleUnsafeResponse = true;
      dataReady = DATA_READY_TRUE;
      break;
    default:
      Component::put(signal);
      break;
  }
}
