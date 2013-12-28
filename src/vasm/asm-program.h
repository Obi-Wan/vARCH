/* 
 * File:   asm_program.h
 * Author: ben
 *
 * Created on 5 agosto 2010, 21.05
 */

#ifndef ASM_PROGRAM_H
#define	ASM_PROGRAM_H

#include "asm-function.h"
#include "exceptions.h"
#include "AsmArgs.h"

#include <deque>

struct asm_program {
  deque<asm_function *> functions;
  vector<asm_data_statement *> shared_vars;
  vector<asm_data_statement *> constants;

  TableOfSymbols globalSymbols;

public:
  asm_program() = default;
  ~asm_program();

  void addFunction(asm_function * _func) {
    functions.insert(functions.end(), _func);
  }
  void addFunctions(list<asm_function *> * _funcs) {
    functions.insert(functions.end(), _funcs->begin(), _funcs->end());
  }

  void addSharedVar(asm_data_statement * _var) {
    shared_vars.insert(shared_vars.end(), _var);
  }
  void addSharedVars(list<asm_data_statement *> && _vars) {
    shared_vars.insert(shared_vars.end(), _vars.begin(), _vars.end());
  }

  void addConstant(asm_data_statement * _var) {
    constants.insert(constants.end(), _var);
  }
  void addConstants(list<asm_data_statement *> && _vars) {
    constants.insert(constants.end(), _vars.begin(), _vars.end());
  }

  size_t getFunciontsTotalSize() const {
    size_t totSize = 0;
    for(asm_function * func : functions) { totSize += func->getSize(); }
    return totSize;
  }
  size_t getSharedVarsTotalSize() const {
    size_t totSize = 0;
    for(asm_data_statement * stmt : shared_vars) { totSize += stmt->getSize(); }
    return totSize;
  }
  size_t getConstantsTotalSize() const {
    size_t totSize = 0;
    for(asm_data_statement * stmt : constants) { totSize += stmt->getSize(); }
    return totSize;
  }

  void assemble(const string & outputName);

  void emitDebugSymbols(const string & outputName) const;
  void emitXMLDebugSymbols(const string & outputName) const;
};

#endif	/* ASM_PROGRAM_H */

