/* 
 * File:   SystemTimer.h
 * Author: ben
 *
 * Created on 3 dicembre 2009, 20.39
 */

#ifndef _SYSTEMTIMER_H
#define	_SYSTEMTIMER_H

#include "Component.h"


class SystemTimer : public Component {
public:

  enum TimerFeatures {
    TIMER_0100_HZ =     (1 <<  0),
    TIMER_0250_HZ =     (1 <<  1),
    TIMER_0300_HZ =     (1 <<  2),
    TIMER_1000_HZ =     (1 <<  3),
  };

  SystemTimer(const int& features);
//  SystemTimer(const SystemTimer& orig);
//  virtual ~SystemTimer();

  void put(const int& signal);
private:

  int timerFeatures;
};

#endif	/* _SYSTEMTIMER_H */

