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

class ParametersHandler {
public:
  static asm_function_param * getParam(const YYLTYPE& pos, asm_arg * reg,
      asm_arg * temp);
};

inline asm_instruction_statement *
InstructionsHandler::getStmt(const YYLTYPE& pos, const int _instr)
{
  switch (_instr) {
    case JSR:
      return new asm_function_call(pos, _instr);
    case RET:
      return new asm_return_statement(pos, _instr);
    default:
      return new asm_instruction_statement(pos, _instr);
  }
}

inline asm_function_param *
ParametersHandler::getParam(const YYLTYPE& pos, asm_arg * reg, asm_arg * temp)
{
  return new asm_function_param( pos, reg, temp );
}

#endif /* IR_LOWLEVEL_PARSERHELPERS_H_ */
