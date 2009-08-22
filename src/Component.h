/* 
 * File:   Component.h
 * Author: ben
 *
 * Created on 20 agosto 2009, 14.53
 */

#ifndef _COMPONENT_H
#define	_COMPONENT_H

class Component {
public:
  Component();
//  Component(const Component& orig);
  virtual ~Component();

  void put(const int& signal);
  int get();

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
private:

  int simpleUnsafeResponse;
};

#endif	/* _COMPONENT_H */

