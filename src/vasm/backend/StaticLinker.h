/*
 * StaticLinker.h
 *
 *  Created on: 02/ago/2011
 *      Author: ben
 */

#ifndef STATICLINKER_H_
#define STATICLINKER_H_

#include "../algorithms/TempsMap.h"
#include "../IR/IR_LowLevel_Arguments.h"
#include "../asm-function.h"
#include "std_istructions.h"

class StaticLinker {

  TempsMap & tempsMap;

  // Push temps map and fin max temp
  uint32_t minNewTemp;

  vector<ListOfStmts::iterator> returns;

  vector<ListOfStmts::iterator> f_calls;

  void scanStmtsAndTemps(ListOfStmts & stmts);
  void mindCalleeSaveRegs(ListOfStmts & stmts);
  void mindReturnStmts(ListOfStmts & stmts);
  void mindParamsFCalls(ListOfStmts & stmts);

public:
  StaticLinker(TempsMap & _tm) : tempsMap(_tm), minNewTemp(FIRST_TEMPORARY) { }

  static uint32_t shiftArgUID(const asm_immediate_arg * arg,
      const bool &isTemp);
  static uint32_t shiftArgUID(const uint32_t & uid, const bool & isTemp);

  static bool shiftedIsSpecialReg(const uint32_t & uid);

  void generateMovesForFunctionCalls(asm_function & function);
};

inline uint32_t
StaticLinker::shiftArgUID(const asm_immediate_arg * arg, const bool & isTemp)
{
  return isTemp * FIRST_TEMPORARY + 1 + arg->content.tempUID;
}

inline uint32_t
StaticLinker::shiftArgUID(const uint32_t & uid, const bool & isTemp)
{
  return isTemp * FIRST_TEMPORARY + 1 + uid;
}

inline bool
StaticLinker::shiftedIsSpecialReg(const uint32_t & uid)
{
  return (uid >= (STACK_POINTER+1) && uid <= (STATE_REGISTER+1));
}

#endif /* STATICLINKER_H_ */
