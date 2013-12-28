/*
 * ObjHandler.h
 *
 *  Created on: 13/feb/2013
 *      Author: ben
 */

#ifndef OBJHANDLER_H_
#define OBJHANDLER_H_

#include "../asm-program.h"

class ObjHandler {
protected:
  string filename;

  ObjHandler(const char * _filename) : filename(_filename) { }
  ObjHandler(const string & _filename) : filename(_filename) { }
};

class ObjLoader : public ObjHandler {
public:
  ObjLoader(const char * _filename)
    : ObjHandler(_filename) { }
  ObjLoader(const string & _filename)
    : ObjHandler(_filename) { }

  void readObj(asm_program & prog);
};

class ObjWriter : public ObjHandler {
public:
  ObjWriter(const char * _filename)
      : ObjHandler(_filename) { }
  ObjWriter(const string & _filename)
      : ObjHandler(_filename) { }

  void writeObj(const asm_program & prog);
};

#endif /* OBJHANDLER_H_ */
