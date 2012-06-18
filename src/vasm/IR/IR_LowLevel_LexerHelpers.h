/*
 * IR_LowLevel_LexerHelpers.h
 *
 *  Created on: 29/lug/2011
 *      Author: ben
 */

#ifndef IR_LOWLEVEL_LEXERHELPERS_H_
#define IR_LOWLEVEL_LEXERHELPERS_H_

#include "IR_LowLevel_Arguments.h"

#include "AST/AST_Low/AST_Low_Stmt.h"

#include <cstdlib>
#include <string.h>

class ArgumentsHandler {
  static uint32_t getRegNum(const char * str);

public:
  static asm_immediate_arg * getReg(const ASTL_ArgRegister * const arg);

  static asm_immediate_arg * getReg(const YYLTYPE & loc, const char * str,
      const TypeOfArgument& type, const ModifierOfArgument & rmt);

  static asm_immediate_arg * getConstInt(const YYLTYPE & loc, const int _val);
  static asm_immediate_arg * getConstReal(const YYLTYPE & loc, const float _val);

  static asm_immediate_arg * getSpecialReg(const YYLTYPE & loc,
      const enum Registers& reg,
      const TypeOfArgument& type, const ModifierOfArgument & rmt);
};

inline asm_immediate_arg *
ArgumentsHandler::getReg(const ASTL_ArgRegister * const arg)
{
  if (arg->getClass() == ASTL_ARG_SPECIAL_REGISTER) {
    const ASTL_ArgSpecialRegister * const sarg = (const ASTL_ArgSpecialRegister *) arg;
    return ArgumentsHandler::getSpecialReg(sarg->pos, sarg->regNum, sarg->kind, sarg->modif);
  } else {
    return ArgumentsHandler::getReg(arg->pos, arg->id.c_str(), arg->kind, arg->modif);
  }
}

inline uint32_t
ArgumentsHandler::getRegNum(const char * str)
{
  size_t len = strlen(str);
  char * tempStr = strndup(str, len);
  uint32_t num = atoi(tempStr);

  free(tempStr);
  return num;
}

inline asm_immediate_arg *
ArgumentsHandler::getReg(const YYLTYPE & loc, const char * str,
    const TypeOfArgument& type, const ModifierOfArgument & rmt)
{
  asm_immediate_arg * tempArg = new asm_immediate_arg(loc);
  tempArg->type = type;
  tempArg->regModType = rmt;
  tempArg->content.val = getRegNum(str+1) + NUM_REGS * (str[0] == 'A');
  tempArg->isTemp = (str[0] == 'T');
  return tempArg;
}

inline asm_immediate_arg *
ArgumentsHandler::getSpecialReg(const YYLTYPE & loc, const enum Registers& reg,
    const TypeOfArgument& type, const ModifierOfArgument & rmt)
{
  asm_immediate_arg * tempArg = new asm_immediate_arg(loc);
  tempArg->type = type;
  tempArg->regModType = rmt;
  tempArg->content.regNum = reg;
  tempArg->isTemp = false;
  return tempArg;
}

inline asm_immediate_arg *
ArgumentsHandler::getConstInt(const YYLTYPE & loc, const int _val)
{
  asm_immediate_arg * tempArg = new asm_immediate_arg(loc);
  tempArg->type = IMMED;
  tempArg->content.val = _val;
  tempArg->regModType = REG_NO_ACTION;
  return tempArg;
}

inline asm_immediate_arg *
ArgumentsHandler::getConstReal(const YYLTYPE & loc, const float _val)
{
  asm_immediate_arg * tempArg = new asm_immediate_arg(loc);
  tempArg->type = IMMED;
  tempArg->content.fval = _val;
  tempArg->regModType = REG_NO_ACTION;
  return tempArg;
}

#endif /* IR_LOWLEVEL_LEXERHELPERS_H_ */
