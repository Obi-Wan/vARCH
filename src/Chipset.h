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
#include "FileHandler.h"
#include <vector>

using namespace std;

class Chipset : public InterruptHandler {
public:
  Chipset(const int& _maxMem, int * _mainMem);
  virtual ~Chipset();

  void startClock();

  void addComponent(Component &);
  //  void addCpu(Cpu &);
  const Cpu& getCpu(int num = 0) const;

  void singlePutToComponent(const int& numComp, const int& signal);
  int singleGetFromComponent(const int& numComp);

private:
  Cpu & cpu;

  int * mainMem;
  const int maxMem;

  static const int bios[];

  vector<Component> components;

  void initMem();

  Bloat loadBiosFromFile(const char * file);
};

#endif	/* _CHIPSET_H */

