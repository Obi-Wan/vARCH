/* 
 * File:   Chipset.h
 * Author: ben
 *
 * Created on 20 agosto 2009, 14.51
 */

#ifndef _CHIPSET_H
#define	_CHIPSET_H

#include "Component.h"
#include "Cpu.h"
//#include <map>

using namespace std;

class Chipset {
public:
  Chipset(const int _maxMem);
  virtual ~Chipset();

  void startClock();

  void addComponent(Component &);
  //  void addCpu(Cpu &);
private:
  Cpu & cpu;

  int * mainMem;

  static const int bios[];
};

#endif	/* _CHIPSET_H */

