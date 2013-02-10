/*
 * ASTL_Tree.h
 *
 *  Created on: 20/ott/2011
 *      Author: ben
 */

#ifndef AST_LOW_TREE_H_
#define AST_LOW_TREE_H_

#include "AST_Low_Function.h"
#include "../../asm-program.h"

class ASTL_Tree {
  list<asm_data_statement *> convertVariables(const vector<ASTL_Stmt *> &)
      const;
  list<asm_statement *> convertStatements(const vector<ASTL_Stmt *> &) const;

public:
  EnvSymbols globalSymbols;

  vector<ASTL_Stmt *> globals;
  vector<ASTL_FunctionDef *> functionDefs;

  ~ASTL_Tree();

  void addFunctionDef(ASTL_FunctionDef * func_def) {
    functionDefs.push_back(func_def);
  }

  void addGlobalVars(list<ASTL_Stmt *> * var_decl) {
    globals.insert(globals.end(), var_decl->begin(), var_decl->end());
  }

  void emitAsm(asm_program & program) const;
  void printTree();
};

#endif /* AST_MID_TREE_H_ */
