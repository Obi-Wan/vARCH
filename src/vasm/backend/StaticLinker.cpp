/*
 * StaticLinker.cpp
 *
 *  Created on: 02/ago/2011
 *      Author: ben
 */

#include "StaticLinker.h"

#include "exceptions.h"

inline void
StaticLinker::scanStmtsAndTemps(ListOfStmts & stmts)
{
  for(ListOfStmts::iterator stmtIt = stmts.begin(); stmtIt != stmts.end();
      stmtIt++)
  {
    asm_statement * stmt = *stmtIt;
    if (stmt->isInstruction()) {
      asm_instruction_statement * i_stmt = (asm_instruction_statement *) stmt;
      vector<asm_arg *> args = i_stmt->args;
      for(size_t numArg = 0; numArg < args.size(); numArg++)
      {
        const bool isTemp = args[numArg]->isTemporary();
        if (isTemp || args[numArg]->isReg()) {
          asm_immediate_arg * arg = (asm_immediate_arg *) args[numArg];
          const uint32_t shiftedTempUID = shiftArgUID(arg, isTemp);
          // to temps map
          tempsMap.putTemp( shiftedTempUID, true);
          // find min avail temp
          if ( (shiftedTempUID+1) > minNewTemp) {
            minNewTemp = shiftedTempUID+1;
            DebugPrintf(("New minNewTemp %u\n", minNewTemp));
          }
        }
      }

      if (stmt->getType() == ASM_RETURN_STATEMENT) {
        DebugPrintf(("Found Return at: %d\n", stmt->position.first_line));
        returns.push_back(stmtIt);
      } else if (stmt->getType() == ASM_FUNCTION_CALL) {
        DebugPrintf(("Found Function call at: %d\n",stmt->position.first_line));
        f_calls.push_back(stmtIt);
      }
    }
  }
}

inline void
StaticLinker::mindCalleeSaveRegs(ListOfStmts & stmts)
{
  for(uint32_t regNum = STD_CALLEE_SAVE; regNum < NUM_REGS; regNum++)
  {
    const uint32_t tempProgr = minNewTemp++;
    tempsMap.putTemp( tempProgr, true);
    {
      const asm_statement * firstStmt = *stmts.begin();
      // Build the move temp[5-8] <- R[5-8]
      asm_instruction_statement * stmt =
          new asm_instruction_statement(firstStmt->position, MOV);

      asm_immediate_arg * regArg = new asm_immediate_arg(firstStmt->position);
      regArg->relative = false;
      regArg->type = REG;
      regArg->content.val = regNum;

      stmt->addArg(regArg);

      asm_immediate_arg * tempArg = new asm_immediate_arg(firstStmt->position);
      tempArg->relative = false;
      tempArg->type = REG;
      tempArg->content.val = tempProgr - FIRST_TEMPORARY - 1;
      tempArg->isTemp = true;

      stmt->addArg(tempArg);

      stmts.push_front(stmt);
    }
    for(vector<ListOfStmts::iterator>::iterator retIt = returns.begin();
        retIt != returns.end(); retIt++)
    {
      asm_return_statement * ret = (asm_return_statement *) **retIt;
      // Build the move R[5-8] <- temp[5-8]
      asm_instruction_statement * stmt =
          new asm_instruction_statement(ret->position, MOV);

      asm_immediate_arg * tempArg = new asm_immediate_arg(ret->position);
      tempArg->relative = false;
      tempArg->type = REG;
      tempArg->content.val = tempProgr - FIRST_TEMPORARY - 1;
      tempArg->isTemp = true;

      stmt->addArg(tempArg);

      asm_immediate_arg * regArg = new asm_immediate_arg(ret->position);
      regArg->relative = false;
      regArg->type = REG;
      regArg->content.val = regNum;

      stmt->addArg(regArg);

      stmts.insert(*retIt, stmt);
    }
  }
}

inline void
StaticLinker::mindReturnStmts(ListOfStmts & stmts)
{
  for(vector<ListOfStmts::iterator>::iterator retIt = returns.begin();
      retIt != returns.end(); retIt++)
  {
    asm_return_statement * ret = (asm_return_statement *) **retIt;
    if (ret->args.size() == 1)
    {
      asm_arg * arg = ret->args[0];
      if (arg->isTemporary() || arg->isReg())
      {
        asm_immediate_arg * i_arg = (asm_immediate_arg *) arg;

        // Build the move R1 <- tempRet
        asm_instruction_statement * stmt =
            new asm_instruction_statement(ret->position, MOV);

        asm_immediate_arg * tempArg = new asm_immediate_arg(ret->position);
        tempArg->relative = i_arg->relative;
        tempArg->type = i_arg->type;
        tempArg->content.val = i_arg->content.val;
        tempArg->isTemp = i_arg->isTemp;

        stmt->addArg(tempArg);

        asm_immediate_arg * regArg = new asm_immediate_arg(ret->position);
        regArg->relative = false;
        regArg->type = REG;
        regArg->content.val = 0;

        stmt->addArg(regArg);

        stmts.insert(*retIt, stmt);
      } else {
        throw WrongArgumentException(
            "Returned element should be a temporary or a register");
      }
    }
  }
}

void
StaticLinker::mindParamsFCalls(ListOfStmts & stmts)
{
  for(vector<ListOfStmts::iterator>::iterator callIt = f_calls.begin();
      callIt != f_calls.end(); callIt++)
  {
    asm_function_call * call = (asm_function_call *) **callIt;
    for(size_t numPar = 1; numPar < call->args.size(); numPar++)
    {
      asm_arg * arg = call->args[numPar];
      if (arg->isTemporary() || arg->isReg())
      {
        asm_immediate_arg * i_arg = (asm_immediate_arg *) arg;

        // Build the move param <- temp
        asm_instruction_statement * stmt =
            new asm_instruction_statement(call->position, MOV);

        asm_immediate_arg * tempArg = new asm_immediate_arg(call->position);
        tempArg->relative = i_arg->relative;
        tempArg->type = i_arg->type;
        tempArg->content.val = i_arg->content.val;
        tempArg->isTemp = i_arg->isTemp;

        stmt->addArg(tempArg);

        asm_immediate_arg * regArg = new asm_immediate_arg(call->position);
        regArg->relative = false;
        regArg->type = REG;
        regArg->content.regNum = call->parameters[numPar-1]->content.regNum;

        stmt->addArg(regArg);

        stmts.insert(*callIt, stmt);
      } else {
        throw WrongArgumentException(
            "Passed argument should be a temporary or a register");
      }
    }
  }
}

void
StaticLinker::generateMovesForFunctionCalls(asm_function & function)
{
  ListOfStmts & stmts = function.stmts;

  // Initial scan
  scanStmtsAndTemps(stmts);

  // Take care of callee-save registers
  // (if it doesn't return, we don't even bother doing callee-save)
  if (returns.size()) {
    mindCalleeSaveRegs(stmts);
  }

  // Move arguments of returns if needed
  mindReturnStmts(stmts);

  // Add moves of parameters for Function calls
  mindParamsFCalls(stmts);
}
