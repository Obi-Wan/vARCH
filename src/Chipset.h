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
  Chipset(const int& _maxMem, int * _mainMem);
  virtual ~Chipset();

  void startClock();

  void addComponent(Component &);
  //  void addCpu(Cpu &);
  const Cpu& getCpu(int num = 0) const;
private:
  Cpu & cpu;

  int * mainMem;
  const int maxMem;

  static const int bios[];
};

#endif	/* _CHIPSET_H */

