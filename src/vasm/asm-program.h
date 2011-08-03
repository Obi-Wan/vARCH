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

#include <deque>

struct asm_program {
  size_t tempOffset;

  deque<asm_function *> functions;
  vector<asm_data_statement *> globals;

  TableOfSymbols globalSymbols;

public:
  asm_program(list<asm_function *> * _funcs,
              list<asm_data_statement *> * _globals) : tempOffset(0)
  {
    /* Now let's copy them, and the data */
    functions.insert(functions.begin(), _funcs->begin(), _funcs->end());
    globals.insert(globals.begin(), _globals->begin(), _globals->end());
  }
  asm_program() : tempOffset(0) { }

  void addFunction(asm_function * _func) {
    functions.insert(functions.end(), _func);
  }
  void addFunctions(list<asm_function *> * _funcs) {
    functions.insert(functions.end(), _funcs->begin(), _funcs->end());
  }

  void addGlobal(asm_data_statement * _global) {
    globals.insert(globals.end(), _global);
  }
  void addGlobals(list<asm_data_statement *> * _globals) {
    globals.insert(globals.end(), _globals->begin(), _globals->end());
  }

  void rebuildFunctionsOffsets() {
    for(size_t i = 0; i < functions.size(); i++) {
      functions[i]->rebuildOffsets();
    }
  }

  int getFunciontsTotalSize() {
    int totSize = 0;
    for(size_t i = 0; i < functions.size(); i++) {
      totSize += functions[i]->getSize();
    }
    return totSize;
  }

  void checkInstructions(const bool & usingTemps) const;
  void assignFunctionParameters();
  void ensureTempsUsage(const bool & used) const;

  void moveMainToTop();
  void addFunctionLabelsToGlobals();
  void assignValuesToLabels();
  void assemble(const string & outputName);

  void emitDebugSymbols(const string & outputName) const;
  void emitXMLDebugSymbols(const string & outputName) const;
};

#endif	/* ASM_PROGRAM_H */

