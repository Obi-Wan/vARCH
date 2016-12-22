/* 
 * File:   Component.h
 * Author: ben
 *
 * Created on 20 agosto 2009, 14.53
 */

#ifndef _COMPONENT_H
#define	_COMPONENT_H

#include "macros.h"

class Component {
public:
  Component()
    : dataReady(DataReady::DATA_READY_FALSE), simpleUnsafeResponse(0)
    { }
  Component(const Component & orig) = default;
  virtual ~Component() = default;

  enum ComponentType : uint8_t {
    COMP_CHAR     = (1u <<  0),
    COMP_BLOCK    = (1u <<  1),
    COMP_KEYBOARD = (1u <<  2),
    COMP_TIMER    = (1u <<  3),
    COMP_CONSOLE  = (1u <<  4),
  };

  enum class ComponentRequestType : uint8_t {
    COMP_TYPE,
    COMP_GET_FEATURES,
    COMP_SET_FEATURES,
  };

  enum DataReady : int8_t {
    DATA_READY_ERROR = (-1),
    DATA_READY_FALSE = 0,
    DATA_READY_TRUE  = 1,
  };

  virtual void put(const ComponentRequestType & request, const int32_t & arg) {
    switch (request) {
      case ComponentRequestType::COMP_TYPE:
        simpleUnsafeResponse = static_cast<int32_t>(ComponentType::COMP_CHAR);
        dataReady = DataReady::DATA_READY_TRUE;
        break;
      case ComponentRequestType::COMP_GET_FEATURES:
        dataReady = DataReady::DATA_READY_ERROR;
        break;
      case ComponentRequestType::COMP_SET_FEATURES:
        dataReady = DataReady::DATA_READY_ERROR;
        break;
    }
  }
  const int32_t & get() {
    dataReady = DataReady::DATA_READY_FALSE;
    return simpleUnsafeResponse;
  }

  const DataReady & isDataReady() { return dataReady; }

protected:
  DataReady dataReady;

  int32_t simpleUnsafeResponse;
};

#endif	/* _COMPONENT_H */

