/* 
 * File:   CharTerminal.cpp
 * Author: ben
 * 
 * Created on 8 dicembre 2009, 21.13
 */

#include <stdio.h>

#include "../include/macros.h"
#include "CharTerminal.h"

CharTerminal::CharTerminal() { }

//CharTerminal::CharTerminal(const CharTerminal& orig) { }
//
//CharTerminal::~CharTerminal() { }


void
CharTerminal::put(const int& signal) {
  DebugPrintf(("CharTerminal: ricevuto un segnale %d\n", signal));
  switch (REQUEST_SHIFT(signal) & REQUEST_TYPE_MASK) {
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
//      WarningPrintf(("No signal recognized"));
      Component::put(signal);
      break;
  }
}
