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
  switch (REQUEST_SHIFT(signal) & REQUEST_TYPE_MASK) {
    case COMP_TYPE:
      simpleUnsafeResponse = COMP_TIMER | COMP_CHAR;
      dataReady = DATA_READY_TRUE;
      break;
    case COMP_GET_FEATURES:
      simpleUnsafeResponse = timerFeatures;
      dataReady = DATA_READY_TRUE;
      break;
    case COMP_SET_FEATURES:
      setTimer(static_cast<TimerFeatures>(signal & REQUEST_ARG_MASK));
      simpleUnsafeResponse = true;
      dataReady = DATA_READY_TRUE;
      break;
  }
}

void
SystemTimer::checkInterruptEvents() {
  if (interruptHandler && timerTimout) {
    timeval timeVal;
    gettimeofday(&timeVal,NULL);
    if ((timeVal.tv_usec - lastTimeCheck) > timerTimout) {
      interruptHandler->interruptSignal(interruptId);
      lastTimeCheck = timeVal.tv_usec;
    }
  }
}

void
SystemTimer::setTimer(const TimerFeatures& timeout) {
  timeval timeVal;
  gettimeofday(&timeVal,NULL);
  lastTimeCheck = timeVal.tv_usec;
  switch (timeout) {
    case TIMER_0100_HZ:
      timerTimout = 10000;
      break;
    case TIMER_0250_HZ:
      timerTimout =  4000;
      break;
    case TIMER_0300_HZ:
      timerTimout =  3333;
      break;
    case TIMER_1000_HZ:
      timerTimout =  1000;
      break;
  }
}