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

#include <sstream>

list<asm_data_statement *>
ASTL_Tree::convertVariables(const vector<ASTL_Stmt *> & variables) const
{
  list<asm_data_statement *> destVars;

  for(ASTL_Stmt * tempStmt : variables)
  {
    switch (tempStmt->getClass()) {
      case ASTL_STMT_LABEL: {
        ASTL_StmtLabel * stmt = (ASTL_StmtLabel *) tempStmt;
        asm_label_statement * tempLabel = new asm_label_statement(stmt->pos, stmt->label);
        tempLabel->is_shared = stmt->shared;
        tempLabel->is_constant = stmt->constant;
        destVars.push_back(tempLabel);
        break;
      }
      case ASTL_STMT_DECL_NUM: {
        ASTL_VarDeclNumber * stmt = (ASTL_VarDeclNumber *) tempStmt;
        int64_t val = atol(stmt->number->number.c_str());
        asm_data_statement * tempDataStmt = new asm_int_keyword_statement(stmt->pos, val, stmt->scale);
        destVars.push_back(tempDataStmt);
        break;
      }
      case ASTL_STMT_DECL_STRING: {
        ASTL_VarDeclString * stmt = (ASTL_VarDeclString *) tempStmt;
        asm_data_statement * tempDataStmt = new asm_string_keyword_statement(stmt->pos, stmt->text);
        destVars.push_back(tempDataStmt);
        break;
      }
      case ASTL_STMT_DECL_FLOAT: {
        throw WrongInstructionException("Floats not supported yet");
//        ASTL_VarDeclFloat * stmt = (ASTL_VarDeclFloat *) tempStmt;
//        float val = atof(stmt->number->number);
      }
      default: {
        throw WrongInstructionException("Unknown data instruction: " + tempStmt->toString());
      }
    }
  }
  return destVars;
}

list<asm_statement *>
ASTL_Tree::convertStatements(const vector<ASTL_Stmt *> & inStmts) const
{
  list<asm_statement *> destStmts;

  for(ASTL_Stmt * tempStmt : inStmts)
  {
    switch (tempStmt->getClass()) {
      case ASTL_STMT_EXP: {
        ASTL_StmtExp * stmt = (ASTL_StmtExp *) tempStmt;
        asm_instruction_statement * asmStmt = InstructionsHandler::getStmt(stmt->pos, stmt->instruction);

        for(ASTL_Arg * t_arg : stmt->args)
        {
          asm_arg * finalArg = NULL;
          switch (t_arg->getClass()) {
            case ASTL_ARG_LABEL: {
              ASTL_ArgLabel * arg = (ASTL_ArgLabel *) t_arg;
              finalArg = new asm_label_arg(arg->pos, arg->label, arg->type, arg->scale);
              break;
            }
            case ASTL_ARG_NUMBER: {
              ASTL_ArgNumber * arg = (ASTL_ArgNumber *) t_arg;
              finalArg = new asm_immediate_arg(arg->pos,
                  atoi(arg->number.c_str()), arg->type, arg->scale, REG_NO_ACTION);
              break;
            }
            case ASTL_ARG_SPECIAL_REGISTER:
            case ASTL_ARG_REGISTER: {
              ASTL_ArgRegister * arg = (ASTL_ArgRegister *) t_arg;
              finalArg = ArgumentsHandler::getReg(arg);

              asm_immediate_arg * tempFinalArg = (asm_immediate_arg *) finalArg;
              if ((arg->kind & DISPLACED) == DISPLACED) {
                DebugPrintf(("        & Displaced\n"));
                tempFinalArg->displacement = atoi(arg->displ->number.c_str());
              }
              if ((arg->kind & INDEXED) == INDEXED) {
                DebugPrintf(("        & Indexed\n"));
                asm_immediate_arg * tempIndex = ArgumentsHandler::getReg(arg);
                tempFinalArg->index = tempIndex->content.tempUID;
                tempFinalArg->isIndexTemp = tempIndex->isTemp;
                delete tempIndex;
              }
              break;
            }
            default: {
              delete asmStmt;
              throw WrongArgumentException("Unknown argument type!");
            }
          }
          asmStmt->addArg(finalArg);
        }
        destStmts.push_back(asmStmt);
        break;
      }
      case ASTL_STMT_LABEL: {
        ASTL_StmtLabel * stmt = (ASTL_StmtLabel *) tempStmt;
        asm_label_statement * asmStmt = new asm_label_statement(stmt->pos, stmt->label);
        /* TODO: remove this forcing to true, and add relative referencing to
         * Program Counter */
        asmStmt->is_constant = stmt->constant || true;
        asmStmt->is_shared = stmt->shared;
        destStmts.push_back(asmStmt);
        break;
      }
      default: {
        throw WrongArgumentException("Unsupported type of stmt");
      }
    }
  }
  return destStmts;
}

void
ASTL_Tree::emitAsm(asm_program & program) const
{
  for(const ASTL_FunctionDef * func : functionDefs)
  {
    asm_function * destFunc = new asm_function(func->pos, func->name);

    // Convert parameters and return
    for(const ASTL_Param * astParam : func->params)
    {
      asm_immediate_arg * srcArg =
          ArgumentsHandler::getReg( astParam->pos, astParam->srcReg.c_str(),
                                    BYTE4, REG, REG_NO_ACTION);
      asm_immediate_arg * destArg =
          ArgumentsHandler::getReg( astParam->pos, astParam->destId.c_str(),
                                    BYTE4, REG, REG_NO_ACTION);

      destFunc->addParameter(ParametersHandler::getParam(astParam->pos, srcArg, destArg));
    }
    // Convert locals
    {
      list<asm_data_statement *> && tempLocals = this->convertVariables(func->locals);
      destFunc->addLocals(move(tempLocals));
    }
    // Convert statements
    {
      list<asm_statement *> && tempStmts = this->convertStatements(func->stmts);
      destFunc->addStmts(move(tempStmts));
    }
    destFunc->finalize();

    program.functions.push_back(destFunc);
  }

  // Convert Globals
  {
    list<asm_data_statement *> && tempGlobals = this->convertVariables(globals);
    program.addGlobals(move(tempGlobals));
  }
}

void
ASTL_Tree::printTree()
{
#ifdef DEBUG
  DebugPrintf(("-- Dumping AST_Low --\n"));
  for(ASTL_FunctionDef * func : functionDefs)
  {
    func->printFunction();
  }
  for(ASTL_Stmt * glob : globals)
  {
    DebugPrintf(("Line: %03d Global: %s\n",
                  glob->pos.first_line, glob->toString().c_str()));
  }
  DebugPrintf(("-- Terminated Dumping AST_Low Code --\n\n"));
#endif
}

ASTL_Tree::~ASTL_Tree()
{
  for(ASTL_FunctionDef * func : functionDefs) { delete func; }
  for(ASTL_Stmt * glob : globals) { delete glob; }
}

