/* 
 * File:   CharTerminal.cpp
 * Author: ben
 * 
 * Created on 8 dicembre 2009, 21.13
 */

#include <stdio.h>

#include "../include/macros.h"
#include "CharTerminal.h"

void
CharTerminal::put(const ComponentRequestType & request, const int32_t& arg)
{
  DebugPrintf(("CharTerminal: got signal %d (arg: %d, %d)\n", request,
                arg, (arg & CHAR_MASK)));
  switch (request) {
    case ComponentRequestType::COMP_TYPE:
      simpleUnsafeResponse = static_cast<int32_t>(ComponentType::COMP_CONSOLE);
      dataReady = DataReady::DATA_READY_TRUE;
      break;
    case ComponentRequestType::COMP_SET_FEATURES:
      printf("%c", (char)(arg & CHAR_MASK));
      simpleUnsafeResponse = true;
      dataReady = DataReady::DATA_READY_TRUE;
      break;
    default:
      InfoPrintf(("Signal not recognized (for CharTerminal): %u", static_cast<uint32_t>(request)));
      Component::put(request, arg);
      break;
  }
}
