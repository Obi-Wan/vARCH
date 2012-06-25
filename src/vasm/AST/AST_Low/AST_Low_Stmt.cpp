/*
 * AST_Low_Stmt.cpp
 *
 *  Created on: 20/ott/2011
 *      Author: ben
 */

#include "AST_Low_Stmt.h"

ASTL_VarDeclNumber::~ASTL_VarDeclNumber()
{
  delete this->number;
}

ASTL_VarDeclFloat::~ASTL_VarDeclFloat()
{
  delete this->number;
}


