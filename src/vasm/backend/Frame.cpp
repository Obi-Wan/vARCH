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

void
Frame::init(asm_function & function)
{
  // Adding registers to TempsMap
  for(uint32_t numReg = 0; numReg < NUM_REGS; numReg++)
  {
    const uint32_t shiftedTempUID = shiftArgUID( numReg, false);
    tempsMap.putTemp( shiftedTempUID, true);
  }

  for(asm_function_param * par : function.parameters)
  {
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
      std::stringstream stream;
      stream << "Double definition of temporary in function parameters "
              << "initialization, at:" << std::endl;
      stream << "  - " << par->position.fileNode->printString()
                << " Line: " << par->position.first_line << ".\n"
                << par->position.fileNode->printStringStackIncludes()
                << std::endl;
      stream << "    (ERROR: " << e.getMessage() << ")" << std::endl;
      throw WrongArgumentException(stream.str());
    }
  }

  for(ListOfStmts::iterator stmtIt = function.stmts.begin();
      stmtIt != function.stmts.end(); stmtIt++)
  {
    asm_statement * stmt = *stmtIt;
    if (stmt->isInstruction()) {
      asm_instruction_statement * i_stmt = (asm_instruction_statement *) stmt;
      std::vector<asm_arg *> args = i_stmt->args;
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

    stmt->addArg(par->source->getCopy());
    stmt->addArg(par->destination->getCopy());

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
      regArg->type = REG;
      regArg->content.val = regNum;

      stmt->addArg(regArg);

      asm_immediate_arg * tempArg = new asm_immediate_arg(firstStmt->position);
      tempArg->type = REG;
      tempArg->content.val = tempProgr - FIRST_TEMPORARY - 1;
      tempArg->isTemp = true;

      stmt->addArg(tempArg);

      stmts.push_front(stmt);
    }
    for(const ListOfStmts::iterator & retIt : returns)
    {
      asm_return_statement * ret = (asm_return_statement *) *retIt;
      // Build the move R[5-8] <- temp[5-8]
      asm_instruction_statement * stmt =
          new asm_instruction_statement(ret->position, MOV);

      asm_immediate_arg * tempArg = new asm_immediate_arg(ret->position);
      tempArg->type = REG;
      tempArg->content.val = tempProgr - FIRST_TEMPORARY - 1;
      tempArg->isTemp = true;

      stmt->addArg(tempArg);

      asm_immediate_arg * regArg = new asm_immediate_arg(ret->position);
      regArg->type = REG;
      regArg->content.val = regNum;

      stmt->addArg(regArg);

      stmts.insert(retIt, stmt);
    }
  }
}

inline void
Frame::mindReturnStmts(ListOfStmts & stmts)
{
  for(const ListOfStmts::iterator & retIt : returns)
  {
    asm_return_statement * ret = (asm_return_statement *) *retIt;
    if (ret->args.size() == 1)
    {
      asm_arg * arg = ret->args[0];

      if (arg->isTemporary() || arg->isReg())
      {
        // Build the move R0 <- tempRet
        asm_instruction_statement * stmt =
            new asm_instruction_statement(ret->position, MOV);

        stmt->addArg(arg->getCopy());

        asm_immediate_arg * regArg = new asm_immediate_arg(ret->position);
        regArg->type = REG;
        regArg->content.regNum = REG_DATA_0;
        regArg->isTemp = false;

        stmt->addArg(regArg);

        stmts.insert(retIt, stmt);
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
  for(ListOfStmts::iterator & callIt : f_calls)
  {
    asm_function_call * call = (asm_function_call *) *callIt;
    for(size_t numPar = 1; numPar < call->args.size(); numPar++)
    {
      asm_arg * arg = call->args[numPar];

      if (arg->isTemporary() || arg->isReg())
      {
        // Build the move param <- temp
        asm_instruction_statement * stmt =
            new asm_instruction_statement(call->position, MOV);

        stmt->addArg(arg->getCopy());
        stmt->addArg(call->parameters[numPar-1]->source->getCopy());

        stmts.insert(callIt, stmt);
      } else {
        throw WrongArgumentException(
            "Passed argument should be a temporary or a register");
      }
    }
  }
}

void
Frame::updateFramePointer(asm_function & function)
{
  const bool isMain = !function.name.compare("main");

  {
    // Add the MOV that initializes FP from SP
    asm_instruction_statement * stmt =
        new asm_instruction_statement(function.position, MOV);

    asm_immediate_arg * regArg1 = new asm_immediate_arg(function.position);
    regArg1->type = REG;
    regArg1->content.regNum = STACK_POINTER;
    regArg1->isTemp = false;

    stmt->addArg(regArg1);

    asm_immediate_arg * regArg2 = new asm_immediate_arg(function.position);
    regArg2->type = REG;
    regArg2->content.regNum = FRAME_POINTER;
    regArg2->isTemp = false;

    stmt->addArg(regArg2);

    function.stmts.push_front(stmt);
  }

  if (!isMain) {
    // If not the main, add the PUSH of the older FP and the POP at the return

    { // PUSH at beginning
      asm_instruction_statement * stmt =
          new asm_instruction_statement(function.position, PUSH);

      asm_immediate_arg * regArg = new asm_immediate_arg(function.position);
      regArg->type = REG;
      regArg->content.regNum = FRAME_POINTER;
      regArg->isTemp = false;

      stmt->addArg(regArg);

      function.stmts.push_front(stmt);
    }
    { // POP at the returns
      asm_instruction_statement * stmt =
          new asm_instruction_statement(function.position, POP);

      asm_immediate_arg * regArg = new asm_immediate_arg(function.position);
      regArg->type = REG;
      regArg->content.regNum = FRAME_POINTER;
      regArg->isTemp = false;

      stmt->addArg(regArg);

      for(ListOfStmts::iterator retIt : returns) {
        function.stmts.insert(retIt, stmt->getCopy());
      }

      delete stmt;
    }
  }
}

void
Frame::generateMovesForFunctionCalls(asm_function & function)
{
  // Move arguments from precolored registers to temporaries
  mindFunctionParameters(function);

  // Take care of callee-save registers
  // (if it doesn't return, we don't even bother doing callee-save)
  if (returns.size()) {
    mindCalleeSaveRegs(function.stmts);
  }

  // Move arguments of returns if needed
  mindReturnStmts(function.stmts);

  // Add moves of parameters for Function calls
  mindParamsFCalls(function.stmts);

  DebugPrintf(("Generating Moves for Function calls, done.\n"));
}

inline asm_immediate_arg *
Frame::makeInitArg(const asm_data_keyword_statement * data,
    const YYLTYPE &pos)
{
  asm_immediate_arg * tempArg = new asm_immediate_arg(pos);
  tempArg->type = IMMED;
  switch (data->getType()) {
    case ASM_INT_KEYWORD_STATEMENT: {
      asm_int_keyword_statement * i_data = (asm_int_keyword_statement *) data;
      tempArg->content.val = i_data->integer;
      break;
    }
    case ASM_REAL_KEYWORD_STATEMENT: {
      asm_real_keyword_statement * i_data = (asm_real_keyword_statement *) data;
      tempArg->content.fval = i_data->real;
      break;
    }
    case ASM_STRING_KEYWORD_STATEMENT: {
      delete tempArg;
      std::stringstream stream;
      stream << "Local string variables not yet supported, at:" << std::endl;
      stream << pos.fileNode->printString()
              << " Line: " << pos.first_line << std::endl;
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
  /// PUSH  $init_var_n
  for(const asm_data_statement * const data : function.stackLocals)
  {
    if (data->getType() == ASM_STRING_KEYWORD_STATEMENT)
    {
      allocateLocalString( (const asm_string_keyword_statement *) data,
                            function.position, function.stmts);
    }
    else if (data->getType() != ASM_LABEL_STATEMENT)
    {
      asm_immediate_arg * tempArg = makeInitArg(
                  (const asm_data_keyword_statement *) data, function.position);

      asm_instruction_statement * stmt =
                        new asm_instruction_statement(function.position, PUSH);

      stmt->addArg(tempArg);
      function.stmts.push_front(stmt);
    }
  }
}

void
Frame::allocateLocalString(const asm_string_keyword_statement * data,
    const YYLTYPE &pos, ListOfStmts & stmts)
{
  ListOfStmts localStmts;
  /* Let's push the string in blocks of 4 chars and in reverse order
   */
  const size_t tStrInitialSize = data->str.size();
  const size_t rest = tStrInitialSize % 4;

  std::vector<int8_t> tempString;
  tempString.insert(tempString.end(), data->str.begin(), data->str.end());
  tempString.resize(tStrInitialSize + (4 - rest) * (rest != 0), '\0');

  ListOfStmts tempListOfStmts;

  for(std::string::const_iterator charIt = data->str.begin(); charIt < data->str.end(); )
  {
    asm_immediate_arg * tempArg = new asm_immediate_arg(pos);
    tempArg->content.val = DEAL_SWORD_FROM_BWORDS(charIt);

    asm_instruction_statement * stmt = new asm_instruction_statement(pos, PUSH);
    stmt->addArg(tempArg);

    tempListOfStmts.push_front(stmt);
  }
  stmts.insert(stmts.begin(), tempListOfStmts.begin(), tempListOfStmts.end());
}

void
Frame::deallocateLocalVariables(asm_function & function)
{
  /// ADD  $tot_vars_size  %SP
  {
    asm_immediate_arg * tempArg = new asm_immediate_arg(function.position);
    tempArg->type = IMMED;
    tempArg->content.val = function.getStackedDataSize();

    asm_immediate_arg * regArg = new asm_immediate_arg(function.position);
    regArg->type = REG;
    regArg->content.regNum = STACK_POINTER;
    regArg->isTemp = false;

    asm_instruction_statement * stmt =
                          new asm_instruction_statement(function.position, ADD);

    stmt->addArg(tempArg);
    stmt->addArg(regArg);

    /// For all the returns, "unwinds" the stack
    for(const ListOfStmts::iterator & retIt : this->returns)
    {
      function.stmts.insert(retIt, stmt->getCopy());
    }
    delete stmt;
  }
}
