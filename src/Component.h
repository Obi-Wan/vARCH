/* 
 * File:   Component.h
 * Author: ben
 *
 * Created on 20 agosto 2009, 14.53
 */

#ifndef _COMPONENT_H
#define	_COMPONENT_H

#define DATA_READY_TRUE     1
#define DATA_READY_FALSE    0
#define DATA_READY_ERROR   -1

class Component {
public:
  Component();
//  Component(const Component& orig);
  virtual ~Component();

  enum ComponentType {
    COMP_CHAR,
    COMP_BLOCK,
    COMP_KEYBOARD,
    COMP_RAM,
    COMP_DISK,
  };

  enum ComponentRequest {
    COMP_TYPE,
    COMP_FEATURES,
  };

  void put(const int& signal);
  int get();

  int isDataReady() { return dataReady; }

  void initInterrupts(const int& priority, const int& id) {
    interruptPriority = priority;
    interruptId = id;
  }
private:
  int dataReady;

  int simpleUnsafeResponse;

  int interruptPriority;
  int interruptId;
};

#endif	/* _COMPONENT_H */

