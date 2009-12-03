/* 
 * File:   SystemTimer.cpp
 * Author: ben
 * 
 * Created on 3 dicembre 2009, 20.39
 */

#include "SystemTimer.h"

SystemTimer::SystemTimer(const int& features) : timerFeatures(features) { }

//SystemTimer::SystemTimer(const SystemTimer& orig) { }

//SystemTimer::~SystemTimer() { }


void
SystemTimer::put(const int& signal) {
  switch (signal) {
    case COMP_TYPE:
      simpleUnsafeResponse = COMP_TIMER | COMP_CHAR;
      dataReady = DATA_READY_TRUE;
      break;
    case COMP_FEATURES:
      simpleUnsafeResponse = timerFeatures;
      dataReady = DATA_READY_TRUE;
      break;
  }
}
