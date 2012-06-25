/*
 * AST_Low_Function.cpp
 *
 *  Created on: 21/ott/2011
 *      Author: ben
 */

#include "AST_Low_Function.h"

ASTL_FunctionDef::~ASTL_FunctionDef()
{
  for(size_t num = 0; num < stmts.size(); num++) { delete stmts[num]; }
  for(size_t num = 0; num < locals.size(); num++) { delete locals[num]; }
}

void
ASTL_FunctionDef::finalize()
{
#ifdef DEBUG
  this->printFunction();
#endif
}

void
ASTL_FunctionDef::printFunction()
{
  DebugPrintf(("Line: %03d Function: %s\n", this->pos.first_line,
                this->name.c_str()));
  for(vector<ASTL_Stmt *>::const_iterator stmt_it = this->stmts.begin();
      stmt_it != this->stmts.end(); stmt_it++)
  {
    const ASTL_Stmt * stmt = *stmt_it;
    DebugPrintf((" Line: %03d %s\n", stmt->pos.first_line,
                  stmt->toString().c_str()));
  }
  for(size_t localNum = 0; localNum < this->locals.size(); localNum++) {
    DebugPrintf((" Line: %03d Local: %s\n",
                  this->locals[localNum]->pos.first_line,
                  this->locals[localNum]->toString().c_str()));
  }
}

