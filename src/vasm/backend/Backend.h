/*
 * Backend.h
 *
 *  Created on: 10/feb/2013
 *      Author: ben
 */

#ifndef BACKEND_H_
#define BACKEND_H_

#include "../asm-program.h"
#include "../AST/AST_Low/AST_Low_Tree.h"
#include "../AsmArgs.h"

class Backend {
  asm_program program;

  const AsmArgs & args;

  void assignFunctionParameters();

  void doRegisterAllocation();
public:
  Backend(const AsmArgs & _args) : args(_args) { }
  Backend(const Backend & other) = default;

  void sourceAST(const ASTL_Tree & tree);

  void emit();

  const asm_program & getProgram() const { return program; };
};

#endif /* BACKEND_H_ */
