/*
 * ASTM_Tree.h
 *
 *  Created on: 20/ott/2011
 *      Author: ben
 */

#ifndef AST_MID_TREE_H_
#define AST_MID_TREE_H_

#include "AST_Mid_Function.h"
#include "../../asm-program.h"

class ASTM_Tree {
public:
  EnvSymbols globalSymbols;

  std::vector<ASTM_VarDecl *> globals;
  std::vector<ASTM_FunctionProto *> functionProtos;
  std::vector<ASTM_FunctionDef *> functionDefs;

  void addFunctionProto(ASTM_FunctionProto * func_proto) {
    functionProtos.push_back(func_proto);
  }
  void addFunctionDef(ASTM_FunctionDef * func_def) {
    functionDefs.push_back(func_def);
  }

  void addGlobalVar(ASTM_VarDecl * var_decl) {
    globals.push_back(var_decl);
  }

  void emitAsm(asm_program & program);
};

#endif /* AST_MID_TREE_H_ */
