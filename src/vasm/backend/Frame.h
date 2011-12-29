/*
 * Frame.h
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

class Frame {

  TempsMap & tempsMap;

  // Push temps map and fin max temp
  uint32_t minNewTemp;

  vector<ListOfStmts::iterator> returns;

  vector<ListOfStmts::iterator> f_calls;
  void mindFunctionParameters(asm_function & function);
  void mindCalleeSaveRegs(ListOfStmts & stmts);
  void mindReturnStmts(ListOfStmts & stmts);
  void mindParamsFCalls(ListOfStmts & stmts);

  asm_immediate_arg * makeInitArg(const asm_data_keyword_statement * data,
      const YYLTYPE &pos);

  void allocateLocalString(const asm_string_keyword_statement * data,
      const YYLTYPE &pos, ListOfStmts & stmts);

public:
  Frame(TempsMap & _tm) : tempsMap(_tm), minNewTemp(FIRST_TEMPORARY) { }

  static uint32_t shiftArgUID(const asm_immediate_arg * arg,
      const bool &isTemp);
  static uint32_t shiftArgUID(const uint32_t & uid, const bool & isTemp);

  static bool shiftedIsSpecialReg(const uint32_t & uid);

  void init(asm_function & function);

  void generateMovesForFunctionCalls(asm_function & function);

  /**
   * Generates the instructions to initialize the local variables on the stack
   * @param function
   *
   * it generates more or less this kind of code:
   *
   * PUSH  $init_var1
   * PUSH  $init_var2
   * PUSH  $init_var3
   * ... [ for all the vars ]
   */
  void allocateLocalVariables(asm_function & function);

  /**
   * Generates the instruction to de-allocate the local variables on the stack
   * @param function
   *
   * it generates more or less this kind of code:
   *
   * ADD  $tot_vars_size  %SP
   */
  void deallocateLocalVariables(asm_function & function);
};

inline uint32_t
Frame::shiftArgUID(const asm_immediate_arg * arg, const bool & isTemp)
{
  return isTemp * FIRST_TEMPORARY + 1 + arg->content.tempUID;
}

inline uint32_t
Frame::shiftArgUID(const uint32_t & uid, const bool & isTemp)
{
  return isTemp * FIRST_TEMPORARY + 1 + uid;
}

inline bool
Frame::shiftedIsSpecialReg(const uint32_t & uid)
{
  return (uid >= (STACK_POINTER+1) && uid < (FIRST_TEMPORARY+1));
}

#endif /* STATICLINKER_H_ */
