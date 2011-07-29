/*
 * IR_LowLevel_ParserHelpers.h
 *
 *  Created on: 29/lug/2011
 *      Author: ben
 */

#ifndef IR_LOWLEVEL_PARSERHELPERS_H_
#define IR_LOWLEVEL_PARSERHELPERS_H_

#include "IR_LowLevel_Statements.h"

class InstructionsHandler {
public:
  static asm_instruction_statement * getStmt(const YYLTYPE& pos,
      const int _instr);
};

inline asm_instruction_statement *
InstructionsHandler::getStmt(const YYLTYPE& pos, const int _instr)
{
  if (_instr == JSR) {
    return new asm_function_call(pos, _instr);
  } else {
    return new asm_instruction_statement(pos, _instr);
  }
}

#endif /* IR_LOWLEVEL_PARSERHELPERS_H_ */
