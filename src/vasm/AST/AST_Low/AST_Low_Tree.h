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

  void convertGlobalVariables(asm_program & prog, const vector<ASTL_Stmt *> &) const;
  void convertFunctionVariables(asm_program & prog, asm_function & func,
      const vector<ASTL_Stmt *> &) const;

public:
  EnvSymbols globalSymbols;

  vector<ASTL_Stmt *> globals;
  vector<ASTL_FunctionDef *> functionDefs;
  vector<ASTL_FunctionProto *> functionProtos;

  ~ASTL_Tree();

  void addFunction(ASTL_FunctionDef * func_def) {
    functionDefs.push_back(func_def);
  }

  void addFunction(ASTL_FunctionProto * func_def) {
    functionProtos.push_back(func_def);
  }

  void addGlobalVars(list<ASTL_Stmt *> * var_decl) {
    globals.insert(globals.end(), var_decl->begin(), var_decl->end());
  }

  void emitAsm(asm_program & program) const;
  void printTree();
};

#endif /* AST_MID_TREE_H_ */
