/*
 * AST_Low_Function.cpp
 *
 *  Created on: 21/ott/2011
 *      Author: ben
 */

#include "AST_Low_Function.h"

ASTL_FunctionDef::~ASTL_FunctionDef()
{
  for(ASTL_Stmt * stmt : stmts) { delete stmt; }
  for(ASTL_Stmt * local : locals) { delete local; }
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
  for(const ASTL_Stmt * const stmt : stmts)
  {
    DebugPrintf((" Line: %03d %s\n", stmt->pos.first_line,
                  stmt->toString().c_str()));
  }
  for(const ASTL_Stmt * const local : locals)
  {
    DebugPrintf((" Line: %03d Local: %s\n", local->pos.first_line,
                  local->toString().c_str()));
  }
}

