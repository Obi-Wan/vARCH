/* 
 * File:   SystemTimer.cpp
 * Author: ben
 * 
 * Created on 3 dicembre 2009, 20.39
 */

#include "SystemTimer.h"

void
SystemTimer::put(const ComponentRequestType & request, const int32_t & arg) {
  switch (request) {
    case ComponentRequestType::COMP_TYPE:
      simpleUnsafeResponse = ComponentType::COMP_TIMER | ComponentType::COMP_CHAR;
      dataReady = DataReady::DATA_READY_TRUE;
      break;
    case ComponentRequestType::COMP_GET_FEATURES:
      simpleUnsafeResponse = timerFeatures;
      dataReady = DataReady::DATA_READY_TRUE;
      break;
    case ComponentRequestType::COMP_SET_FEATURES:
      setTimer(static_cast<TimerFeatures>(arg));
      simpleUnsafeResponse = true;
      dataReady = DataReady::DATA_READY_TRUE;
      break;
  }
}

void
SystemTimer::checkInterruptEvents() {
  if (interruptHandler && timerTimeout) {
    timeval timeVal;
    gettimeofday(&timeVal, nullptr);
    if ((timeVal.tv_usec - lastTimeCheck) > timerTimeout) {
      interruptHandler->interruptSignal(interruptId);
      lastTimeCheck = timeVal.tv_usec;
    }
  }
}

void
SystemTimer::setTimer(const TimerFeatures & timeout) {
  timeval timeVal;
  gettimeofday(&timeVal, nullptr);
  lastTimeCheck = timeVal.tv_usec;
  switch (timeout) {
    case TimerFeatures::TIMER_0100_HZ:
      timerTimeout = 10000;
      break;
    case TimerFeatures::TIMER_0250_HZ:
      timerTimeout =  4000;
      break;
    case TimerFeatures::TIMER_0300_HZ:
      timerTimeout =  3333;
      break;
    case TimerFeatures::TIMER_1000_HZ:
      timerTimeout =  1000;
      break;
  }
}
