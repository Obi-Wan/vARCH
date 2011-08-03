/*
 * Disassembler.h
 *
 *  Created on: 02/ago/2011
 *      Author: ben
 */

#ifndef DISASSEMBLER_H_
#define DISASSEMBLER_H_

#include "../asm-program.h"

class Disassembler {
  void printArg(const int & typeArg, const int & arg);
public:
//  Disassembler();

  void disassembleAndPrint(const Bloat & bytecode);
};

#endif /* DISASSEMBLER_H_ */
