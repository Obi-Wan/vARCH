/*
 * IR_LowLevel_LexerHelpers.h
 *
 *  Created on: 29/lug/2011
 *      Author: ben
 */

#ifndef IR_LOWLEVEL_LEXERHELPERS_H_
#define IR_LOWLEVEL_LEXERHELPERS_H_

#include "IR_LowLevel_Arguments.h"

#include <cstdlib>
#include <string.h>

#define L_SHIFT_TXT_REL(yytext, offset) \
  (yytext + ( (*(yytext+offset) == 'r') ? (offset+3) : (offset+2) ) )
#define L_SHIFT_TXT_REL_0(yytext) L_SHIFT_TXT_REL(yytext, 0)
#define L_SHIFT_TXT_REL_1(yytext) L_SHIFT_TXT_REL(yytext, 1)

#define L_IS_REL(yytext, offset) \
  (*(yytext+offset) == 'r')
#define L_IS_REL_0(yytext) L_IS_REL(yytext, 0)
#define L_IS_REL_1(yytext) L_IS_REL(yytext, 1)

class ArgumentsHandler {
  static int getRegNum(const char * str);
  static uint32_t getTempNum(const char * str, const TypeOfArgument& type);

public:
  static void getReg(asm_arg *& arg, YYLTYPE *& loc, const char * str,
      const TypeOfArgument& type, const int& offset, const bool rel);
  static void getTemporary(asm_arg *& arg, YYLTYPE *& loc, const char * str,
      const TypeOfArgument& type, const bool rel);

  static void getConstInt(asm_arg *& arg, YYLTYPE *& loc, const int _val);
  static void getConstReal(asm_arg *& arg, YYLTYPE *& loc, const float _val);

  static void getSpecialReg(asm_arg *& arg, YYLTYPE *& loc,
      const enum Registers& reg);
  static void getSpecialAddrReg(asm_arg *& arg, YYLTYPE *& loc,
      const enum Registers& reg, const bool rel);
};

inline int
ArgumentsHandler::getRegNum(const char * str)
{
  char * tempStr = strndup(str,1);
  int num = atoi(tempStr) -1;
  free(tempStr);
  return num;
}

inline uint32_t
ArgumentsHandler::getTempNum(const char * str, const TypeOfArgument& type)
{
  size_t len = strlen(str);

  switch (type) {
    case REG_POST_INCR:
    case REG_POST_DECR: {
      len -= 1;
      break;
    }
    case ADDR_POST_INCR:
    case ADDR_POST_DECR:
    case ADDR_IN_REG_POST_INCR:
    case ADDR_IN_REG_POST_DECR: {
      len -= 2;
      break;
    }
    default: {
      break;
    }
  }
  char * tempStr = strndup(str, len);
  uint32_t num = atoi(tempStr);

  free(tempStr);
  return num;
}

inline void
ArgumentsHandler::getReg(asm_arg *& arg, YYLTYPE *& loc, const char * str,
    const TypeOfArgument& type, const int& offset, const bool rel)
{
  asm_immediate_arg * tempArg = new asm_immediate_arg(*loc);
  tempArg->relative = rel;
  tempArg->type = type;
  tempArg->content.val = getRegNum(str) + offset;
  arg = tempArg;
}

inline void
ArgumentsHandler::getTemporary(asm_arg *& arg, YYLTYPE *& loc,
    const char * str, const TypeOfArgument& type, const bool rel)
{
  asm_immediate_arg * tempArg = new asm_immediate_arg(*loc);
  tempArg->relative = rel;
  tempArg->type = type;
  tempArg->content.tempUID = getTempNum(str, type);
  tempArg->isTemp = true;
  arg = tempArg;
}

inline void
ArgumentsHandler::getSpecialReg(asm_arg *& arg, YYLTYPE *& loc,
    const enum Registers& reg)
{
  asm_immediate_arg * tempArg = new asm_immediate_arg(*loc);
  tempArg->type = REG;
  tempArg->content.regNum = reg;
  tempArg->relative = false;
  arg = tempArg;
}

inline void
ArgumentsHandler::getSpecialAddrReg(asm_arg *& arg, YYLTYPE *& loc,
    const enum Registers& reg, const bool rel)
{
  asm_immediate_arg * tempArg = new asm_immediate_arg(*loc);
  tempArg->type = ADDR_IN_REG;
  tempArg->content.regNum = reg;
  tempArg->relative = rel;
  arg = tempArg;
}

inline void
ArgumentsHandler::getConstInt(asm_arg *& arg, YYLTYPE *& loc,
    const int _val)
{
  asm_immediate_arg * tempArg = new asm_immediate_arg(*loc);
  tempArg->type = CONST;
  tempArg->content.val = _val;
  tempArg->relative = false;
  arg = tempArg;
}

inline void
ArgumentsHandler::getConstReal(asm_arg *& arg, YYLTYPE *& loc,
    const float _val)
{
  asm_immediate_arg * tempArg = new asm_immediate_arg(*loc);
  tempArg->type = CONST;
  tempArg->content.fval = _val;
  tempArg->relative = false;
  arg = tempArg;
}

#endif /* IR_LOWLEVEL_LEXERHELPERS_H_ */
