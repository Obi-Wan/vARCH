/*
 * Optimizer.cpp
 *
 *  Created on: 02/ago/2011
 *      Author: ben
 */

#include "Optimizer.h"

void
Optimizer::removeUselessMoves(asm_function & func)
{
  // Remove Moves with identical source and destination

  // Find useless statements
  vector<ListOfStmts::iterator> idMoves;

  DebugPrintf(("Trying to find useless moves\n"));
  for(ListOfStmts::iterator stmtIt = func.stmts.begin();
      stmtIt != func.stmts.end(); stmtIt++)
  {
    asm_statement * stmt = *stmtIt;
    if (stmt->getType() == ASM_INSTRUCTION_STATEMENT) {
      asm_instruction_statement * i_stmt = (asm_instruction_statement *) stmt;
      if (i_stmt->instruction == MOV) {
        DebugPrintf(("Found Move!\n"));
        vector<asm_arg *> args = i_stmt->args;
        if (args[0]->getType() == ASM_IMMEDIATE_ARG
            && args[1]->getType() == ASM_IMMEDIATE_ARG)
        {
          asm_immediate_arg * arg0 = (asm_immediate_arg *) args[0];
          asm_immediate_arg * arg1 = (asm_immediate_arg *) args[1];
          DebugPrintf(("Move between immediate args (%d, %d)!\n",
              arg0->content.val, arg1->content.val));

          if (arg0->content.val == arg1->content.val
              && arg0->isTemp == arg1->isTemp
              && arg0->type == arg1->type)
          {
            DebugPrintf(("Found useless MOV to remove\n"));
            idMoves.push_back(stmtIt);
          }
        }
      }
    }
  }
  for(size_t numStmt = 0; numStmt < idMoves.size(); numStmt++)
  {
    func.stmts.erase(idMoves[numStmt]);
  }
}

void
Optimizer::removeUselessArithemtics(asm_function & func)
{
  // Remove Arithmetic operations that do nothing

  // Find useless statements
  vector<ListOfStmts::iterator> useless;

  DebugPrintf(("Trying to find useless arithmetic operations\n"));
  for(ListOfStmts::iterator stmtIt = func.stmts.begin();
      stmtIt != func.stmts.end(); stmtIt++)
  {
    asm_statement * stmt = *stmtIt;
    if (stmt->getType() == ASM_INSTRUCTION_STATEMENT) {
      asm_instruction_statement * i_stmt = (asm_instruction_statement *) stmt;
      vector<asm_arg *> args = i_stmt->args;
      switch (i_stmt->instruction) {
        case SUB:
        case ADD: {
          if (args[0]->getType() == ASM_IMMEDIATE_ARG) {
            asm_immediate_arg * arg0 = (asm_immediate_arg *) args[0];

            if (arg0->type == COST && arg0->content.val == 0) {
              DebugPrintf(("Found useless ADD or SUB to remove\n"));
              useless.push_back(stmtIt);
            }
          }
          break;
        }
        case MULT:
        case DIV: {
          if (args[0]->getType() == ASM_IMMEDIATE_ARG) {
            asm_immediate_arg * arg0 = (asm_immediate_arg *) args[0];

            if (arg0->type == COST && arg0->content.val == 1) {
              DebugPrintf(("Found useless MULT or DIV to remove\n"));
              useless.push_back(stmtIt);
            }
          }
          break;
        }
        default: break;
      }
    }
  }
  for(size_t numStmt = 0; numStmt < useless.size(); numStmt++)
  {
    func.stmts.erase(useless[numStmt]);
  }
}

