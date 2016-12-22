/* 
 * File:   CharTerminal.h
 * Author: ben
 *
 * Created on 8 dicembre 2009, 21.13
 */

#ifndef _CHARTERMINAL_H
#define	_CHARTERMINAL_H

#include "Component.h"

#define CHAR_MASK 0xff

class CharTerminal : public Component {
public:
  CharTerminal();
//  CharTerminal(const CharTerminal& orig);
//  virtual ~CharTerminal();

  void put(const ComponentRequestType & request, const int32_t & arg);
private:

};

#endif	/* _CHARTERMINAL_H */

