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
#include "../include/masks.h"

#include <vector>

#define EXTRACT_REQUEST(x) (EXTRACT_HIGHER_HWORD_FROM_SWORD(x))
#define EXTRACT_DEVICE(x)  (EXTRACT_LOWER__HWORD_FROM_SWORD(x))

class Chipset : public Component {
public:
  Chipset(const uint32_t & _maxMem, DoubleWord * _mainMem);
  virtual ~Chipset() = default;

  void startClock();

  void addComponent(Component * comp) { components.push_back(comp); }

  //  void addCpu(Cpu &);
  const Cpu & getCpu(const uint32_t & num = 0) const { return cpu; }

  void singlePutToComponent(const uint32_t & numComp, const uint32_t & signal);
  int32_t singleGetFromComponent(const uint32_t & numComp);

  virtual void put(const ComponentRequestType & request, const int32_t & arg);

private:
  Cpu & cpu;

  DoubleWord * mainMem;
  const uint32_t maxMem;

  std::vector<Component *> components;

  void initMem();

  Bloat loadBiosFromFile(const char * file);
};

#endif	/* _CHIPSET_H */

