/*
 * Frame.cpp
 *
 *  Created on: 02/ago/2011
 *      Author: ben
 */

#include "Frame.h"

#include "exceptions.h"
#include "../IncludesTree.h"

#include <sstream>

using namespace std;

void
Frame::init(asm_function & function)
{
  ListOfStmts & stmts = function.stmts;
  ListOfParams & params = function.parameters;

  // Adding registers to TempsMap
  for(uint32_t numReg = 0; numReg < NUM_REGS * 2; numReg++)
  {
    const uint32_t shiftedTempUID = shiftArgUID( numReg, false);
    tempsMap.putTemp( shiftedTempUID, true);
  }

  for(ListOfParams::iterator parIt = params.begin(); parIt != params.end();
      parIt++)
  {
    asm_function_param * par = *parIt;
    try {
      asm_immediate_arg * temp = (asm_immediate_arg *) par->destination;
      const uint32_t shiftedTempUID = shiftArgUID(temp->content.tempUID, true);
      // to temps map
      tempsMap.putTemp( shiftedTempUID, false);
      // find min avail temp
      if ( (shiftedTempUID+1) > minNewTemp) {
        minNewTemp = shiftedTempUID+1;
        DebugPrintf(("New minNewTemp %u\n", minNewTemp));
      }
    } catch (const BasicException & e) {
      stringstream stream;
      stream << "Double definition of temporary in function parameters "
              << "initialization, at:" << endl;
      stream << "  - " << par->position.fileNode->printString()
                << " Line: " << par->position.first_line << ".\n"
                << par->position.fileNode->printStringStackIncludes()
                << endl;
      throw WrongArgumentException(stream.str());
    }
  }

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
  DebugPrintf(("Looking for notable Statements, done.\n"));
}

inline void
Frame::mindFunctionParameters(asm_function & function)
{
  ListOfStmts & stmts = function.stmts;
  ListOfParams & params = function.parameters;

  for(ListOfParams::reverse_iterator parIt = params.rbegin();
      parIt != params.rend(); ++parIt)
  {
    asm_function_param * par = *parIt;

    // Build the move temp_n <- R_m
    asm_instruction_statement * stmt =
        new asm_instruction_statement(par->position, MOV);

    stmt->addArg(par->source);
    stmt->addArg(par->destination);

    stmts.push_front(stmt);
  }
}

inline void
Frame::mindCalleeSaveRegs(ListOfStmts & stmts)
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
Frame::mindReturnStmts(ListOfStmts & stmts)
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
Frame::mindParamsFCalls(ListOfStmts & stmts)
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
        stmt->addArg(call->parameters[numPar-1]->source);

        stmts.insert(*callIt, stmt);
      } else {
        throw WrongArgumentException(
            "Passed argument should be a temporary or a register");
      }
    }
  }
}

void
Frame::generateMovesForFunctionCalls(asm_function & function)
{
  ListOfStmts & stmts = function.stmts;

  // Move arguments from precolored registers to temporaries
  mindFunctionParameters(function);

  // Take care of callee-save registers
  // (if it doesn't return, we don't even bother doing callee-save)
  if (returns.size()) {
    mindCalleeSaveRegs(stmts);
  }

  // Move arguments of returns if needed
  mindReturnStmts(stmts);

  // Add moves of parameters for Function calls
  mindParamsFCalls(stmts);

  DebugPrintf(("Generating Moves for Function calls, done.\n"));
}

inline asm_immediate_arg *
Frame::makeInitArg(const asm_data_keyword_statement * data,
    const YYLTYPE &pos)
{
  asm_immediate_arg * tempArg = new asm_immediate_arg(pos);
  tempArg->relative = false;
  tempArg->type = CONST;
  switch (data->getType()) {
    case ASM_INT_KEYWORD_STATEMENT: {
      asm_int_keyword_statement * i_data = (asm_int_keyword_statement *) data;
      tempArg->content.val = i_data->integer;
      break;
    }
    case ASM_LONG_KEYWORD_STATEMENT: {
      asm_long_keyword_statement * i_data = (asm_long_keyword_statement *) data;
      tempArg->content.lval = i_data->longInteger;
      break;
    }
    case ASM_REAL_KEYWORD_STATEMENT: {
      asm_real_keyword_statement * i_data = (asm_real_keyword_statement *) data;
      tempArg->content.fval = i_data->real;
      break;
    }
    case ASM_STRING_KEYWORD_STATEMENT: {
      delete tempArg;
      stringstream stream;
      stream << "Local string variables not yet supported, at:" << endl;
      stream << pos.fileNode->printString()
              << " Line: " << pos.first_line << endl;
      throw WrongArgumentException(stream.str());
    }
    default:
      break;
  }
  return tempArg;
}

void
Frame::allocateLocalVariables(asm_function & function)
{
  ListOfStmts & stmts = function.stmts;
  const ListOfDataStmts & localVars = function.stackLocals;

  /// PUSH  $init_var_n
  for(ListOfDataStmts::const_iterator initIt = localVars.begin();
      initIt != localVars.end(); initIt++)
  {
    const asm_data_statement * data = *initIt;
    if (data->getType() == ASM_STRING_KEYWORD_STATEMENT)
    {
      allocateLocalString( (const asm_string_keyword_statement *) data,
                            function.position, stmts);
    }
    else if (data->getType() != ASM_LABEL_STATEMENT)
    {
      asm_immediate_arg * tempArg = makeInitArg(
                  (const asm_data_keyword_statement *) data, function.position);

      asm_instruction_statement * stmt =
                        new asm_instruction_statement(function.position, PUSH);

      stmt->addArg(tempArg);
      stmts.push_front(stmt);
    }
  }
}

void
Frame::allocateLocalString(const asm_string_keyword_statement * data,
    const YYLTYPE &pos, ListOfStmts & stmts)
{
  ListOfStmts localStmts;
  for(string::const_iterator charIt = data->str.begin();
      charIt != data->str.end(); charIt++)
  {
    asm_immediate_arg * tempArg = new asm_immediate_arg(pos);
    tempArg->content.val = *charIt;

    asm_instruction_statement * stmt = new asm_instruction_statement(pos, PUSH);
    stmt->addArg(tempArg);

    localStmts.push_back(stmt);
  }
  stmts.insert(stmts.begin(), localStmts.begin(), localStmts.end());
}

void
Frame::deallocateLocalVariables(asm_function & function)
{
  ListOfStmts & stmts = function.stmts;

  const size_t & stackPointerBias = function.getStackedDataSize();

  /// ADD  $tot_vars_size  %SP
  {
    asm_immediate_arg * tempArg = new asm_immediate_arg(function.position);
    tempArg->relative = false;
    tempArg->type = CONST;
    tempArg->content.val = stackPointerBias;

    asm_immediate_arg * regArg = new asm_immediate_arg(function.position);
    regArg->relative = false;
    regArg->type = REG;
    regArg->content.regNum = STACK_POINTER;
    regArg->isTemp = false;

    asm_instruction_statement * stmt =
                          new asm_instruction_statement(function.position, ADD);

    stmt->addArg(tempArg);
    stmt->addArg(regArg);

    /// For all the returns, "unwinds" the stack
    for(vector<ListOfStmts::iterator>::iterator retIt = returns.begin();
        retIt != returns.end(); retIt++)
    {
      stmts.insert(*retIt, stmt);
    }
  }
}
