/*
 * ASTL_Tree.cpp
 *
 *  Created on: 20/ott/2011
 *      Author: ben
 */

#include "AST_Low_Tree.h"

#include "IR/IR_LowLevel_LexerHelpers.h"
#include "IR/IR_LowLevel_ParserHelpers.h"

#include "exceptions.h"

list<asm_data_statement *> *
ASTL_Tree::convertVariables(const vector<ASTL_Stmt *> & variables) const
{
  list<asm_data_statement *> * destVars = new list<asm_data_statement *>();
  for(size_t varNum = 0; varNum < variables.size(); varNum++) {
    ASTL_Stmt * tempStmt = variables[varNum];
    switch (tempStmt->getClass()) {
      case ASTL_STMT_LABEL: {
        ASTL_StmtLabel * stmt = (ASTL_StmtLabel *) tempStmt;
        asm_label_statement * tempLabel = new asm_label_statement(stmt->pos, stmt->label);
        tempLabel->is_shared = stmt->shared;
        tempLabel->is_constant = stmt->constant;
        destVars->push_back(tempLabel);
        break;
      }
      case ASTL_STMT_DECL_NUM: {
        ASTL_VarDeclNumber * stmt = (ASTL_VarDeclNumber *) tempStmt;
        asm_data_statement * tempDataStmt = NULL;
        switch (stmt->scale) {
          case BYTE1: {
            uint8_t val = atoi(stmt->number->number.c_str());
            tempDataStmt = new asm_char_keyword_statement(stmt->pos, (const char *)&val);
          }
          case BYTE2: {
            uint16_t val = atoi(stmt->number->number.c_str());
            tempDataStmt = new asm_int_keyword_statement(stmt->pos, val);
          }
          case BYTE4: {
            uint32_t val = atoi(stmt->number->number.c_str());
            tempDataStmt = new asm_int_keyword_statement(stmt->pos, val);
          }
          case BYTE8: {
            uint32_t val = atol(stmt->number->number.c_str());
            tempDataStmt = new asm_int_keyword_statement(stmt->pos, val);
          }
          default: {
            throw WrongArgumentException("Wrong Number scale!");
          }
        }
        destVars->push_back(tempDataStmt);
        break;
      }
      case ASTL_STMT_DECL_STRING: {
        ASTL_VarDeclString * stmt = (ASTL_VarDeclString *) tempStmt;
        asm_data_statement * tempDataStmt = new asm_string_keyword_statement(stmt->pos, stmt->text);
        destVars->push_back(tempDataStmt);
        break;
      }
      case ASTL_STMT_DECL_FLOAT: {
        delete destVars;
        throw WrongInstructionException("Floats not supported yet");
//        ASTL_VarDeclFloat * stmt = (ASTL_VarDeclFloat *) tempStmt;
//        float val = atof(stmt->number->number);
      }
      default: {
        delete destVars;
        throw WrongInstructionException("Unknown data instruction: " + tempStmt->toString());
      }
    }
  }
  return destVars;
}

list<asm_statement *> *
ASTL_Tree::convertStatements(const vector<ASTL_Stmt *> & inStmts) const
{
  list<asm_statement *> * destStmts = new list<asm_statement *>();
  for(size_t stmtNum = 0; stmtNum < inStmts.size(); stmtNum++)
  {
    ASTL_Stmt * tempStmt = inStmts[stmtNum];
    switch (tempStmt->getClass()) {
      case ASTL_STMT_EXP: {
        ASTL_StmtExp * stmt = (ASTL_StmtExp *) tempStmt;
        asm_instruction_statement * asmStmt = InstructionsHandler::getStmt(stmt->pos, stmt->instruction);

        for(size_t argNum = 0; argNum < stmt->args.size(); argNum++)
        {
          ASTL_Arg * t_arg = stmt->args[argNum];
          asm_arg * finalArg = NULL;
          switch (t_arg->getClass()) {
            case ASTL_ARG_LABEL: {
              ASTL_ArgLabel * arg = (ASTL_ArgLabel *) t_arg;
              finalArg = new asm_label_arg(arg->pos, arg->label, arg->type);
              break;
            }
            case ASTL_ARG_NUMBER: {
              ASTL_ArgNumber * arg = (ASTL_ArgNumber *) t_arg;
              finalArg = new asm_immediate_arg(arg->pos,
                  atoi(arg->number.c_str()), IMMED, stmt->scale, REG_NO_ACTION);
              break;
            }
            case ASTL_ARG_SPECIAL_REGISTER:
            case ASTL_ARG_REGISTER: {
              ASTL_ArgRegister * arg = (ASTL_ArgRegister *) t_arg;
              finalArg = ArgumentsHandler::getReg(arg);

              asm_immediate_arg * tempFinalArg = (asm_immediate_arg *) finalArg;
              if (arg->kind | DISPLACED) {
                tempFinalArg->displacement = atoi(arg->displ->number.c_str());
              }
              if (arg->kind | INDEXED) {
                asm_immediate_arg * tempIndex = ArgumentsHandler::getReg(arg);
                tempFinalArg->index = tempIndex->content.tempUID;
                tempFinalArg->isIndexTemp = tempIndex->isTemp;
                delete tempIndex;
              }
            }
            default: {
              delete asmStmt;
              delete destStmts;
              throw WrongArgumentException("Unknown argument type!");
            }
          }
          asmStmt->addArg(finalArg);
        }
        destStmts->push_back(asmStmt);
        break;
      }
      case ASTL_STMT_LABEL: {
        ASTL_StmtLabel * stmt = (ASTL_StmtLabel *) tempStmt;
        asm_label_statement * asmStmt = new asm_label_statement(stmt->pos, stmt->label);
        asmStmt->is_constant = stmt->constant;
        asmStmt->is_shared = stmt->shared;
        destStmts->push_back(asmStmt);
        break;
      }
      default: {
        delete destStmts;
        throw WrongArgumentException("Unsupported type of stmt");
      }
    }
  }
  return destStmts;
}

void
ASTL_Tree::emitAsm(asm_program & program)
{
  for(size_t funcNum = 0; funcNum < functionDefs.size(); funcNum++) {
    ASTL_FunctionDef & func = *functionDefs[funcNum];
    asm_function * destFunc = new asm_function(func.pos, func.name);

    // Convert parameters and return
    {
      for(size_t paramNum = 0; paramNum < func.params.size(); paramNum++)
      {
        ASTL_Param * astParam = func.params[paramNum];

        asm_immediate_arg * srcArg =
            ArgumentsHandler::getReg(astParam->pos, astParam->srcReg.c_str(), REG, REG_NO_ACTION);
        asm_immediate_arg * destArg =
            ArgumentsHandler::getReg(astParam->pos, astParam->destId.c_str(), REG, REG_NO_ACTION);

        destFunc->addParameter(ParametersHandler::getParam(astParam->pos, srcArg, destArg));
      }
    }
    // Convert locals
    {
      list<asm_data_statement *> * tempLocals = this->convertVariables(func.locals);
      destFunc->addLocals(tempLocals);
      delete tempLocals;
    }
    // Convert statements
    {
      list<asm_statement *> * tempStmts = this->convertStatements(func.stmts);
      destFunc->addStmts(tempStmts);
      delete tempStmts;
    }
    destFunc->finalize();

    program.functions.push_back(destFunc);
  }

  // Convert Globals
  {
    list<asm_data_statement *> * tempGlobals = this->convertVariables(globals);
    program.addGlobals(tempGlobals);
    delete tempGlobals;
  }
}

void
ASTL_Tree::printTree()
{
#ifdef DEBUG
  DebugPrintf(("-- Dumping AST_Low --\n"));
  for(size_t funcNum = 0; funcNum < functionDefs.size(); funcNum++) {
    ASTL_FunctionDef & func = *functionDefs[funcNum];
    DebugPrintf(("Line: %03d Function: %s\n", func.pos.first_line,
                  func.name.c_str()));
    for(vector<ASTL_Stmt *>::const_iterator stmt_it = func.stmts.begin();
        stmt_it != func.stmts.end(); stmt_it++)
    {
      const ASTL_Stmt * stmt = *stmt_it;
      DebugPrintf((" Line: %03d %s\n", stmt->pos.first_line,
                    stmt->toString().c_str()));
    }
    for(size_t localNum = 0; localNum < func.locals.size(); localNum++) {
      DebugPrintf((" Line: %03d Local: %s\n",
                    func.locals[localNum]->pos.first_line,
                    func.locals[localNum]->toString().c_str()));
    }
  }
  for(size_t num = 0; num < globals.size(); num++) {
    DebugPrintf(("Line: %03d Global: %s\n",
                  globals[num]->pos.first_line,
                  globals[num]->toString().c_str()));
  }
  DebugPrintf(("-- Terminated Dumping AST_Low Code --\n\n"));
#endif
}
