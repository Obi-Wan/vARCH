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
CharTerminal::put(const ComponentRequestType & request, const int32_t& arg)
{
  DebugPrintf(("CharTerminal: got signal %d (arg: %d, %d)\n", request,
                arg, (arg & CHAR_MASK)));
  switch (request) {
    case ComponentRequestType::COMP_TYPE:
      simpleUnsafeResponse = COMP_CONSOLE;
      dataReady = DataReady::DATA_READY_TRUE;
      break;
    case ComponentRequestType::COMP_SET_FEATURES:
      printf("%c", (char)(arg & CHAR_MASK));
      simpleUnsafeResponse = true;
      dataReady = DataReady::DATA_READY_TRUE;
      break;
    default:
//      WarningPrintf(("No signal recognized"));
      Component::put(request, arg);
      break;
  }
}
