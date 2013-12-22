/*
 * ObjHandler.h
 *
 *  Created on: 13/feb/2013
 *      Author: ben
 */

#ifndef OBJHANDLER_H_
#define OBJHANDLER_H_

#include "../../FileHandler.h"
#include "../asm-program.h"

class ObjLoader : public FileHandler {
public:
  ObjLoader(const char * filename)
    : FileHandler(filename, ios_base::in) { }
  ObjLoader(const string & filename)
    : FileHandler(filename, ios_base::in) { }

  void readObj(asm_program & prog);
};

class ObjWriter : public FileHandler {
public:
  ObjWriter(const char * filename)
      : FileHandler(filename, ios_base::out | ios_base::trunc ) { }
  ObjWriter(const string & filename)
      : FileHandler(filename.c_str(), ios_base::out | ios_base::trunc ) { }

  void writeObj(const asm_program & prog);
};

#endif /* OBJHANDLER_H_ */
