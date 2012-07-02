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
  vector<asm_data_statement *> globals;

  TableOfSymbols globalSymbols;

public:
  asm_program(list<asm_function *> * _funcs,
              list<asm_data_statement *> * _globals)
  {
    /* Now let's copy them, and the data */
    functions.insert(functions.begin(), _funcs->begin(), _funcs->end());
    globals.insert(globals.begin(), _globals->begin(), _globals->end());
  }
  asm_program() { }
  ~asm_program();

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

  void rebuildOffsets();

  size_t getFunciontsTotalSize() const {
    size_t totSize = 0;
    for(size_t i = 0; i < functions.size(); i++) {
      totSize += functions[i]->getSize();
    }
    return totSize;
  }
  size_t getGlobalsTotalSize() const {
    size_t totSize = 0;
    for(size_t i = 0; i < globals.size(); i++) {
      totSize += globals[i]->getSize();
    }
    return totSize;
  }

  void checkInstructions(const bool & usingTemps) const;
  void assignFunctionParameters();
  void ensureTempsUsage(const bool & used) const;

  void moveMainToTop();
  void doRegisterAllocation(const AsmArgs & args);
  void exposeGlobalLabels();
  void assignValuesToLabels();
  void assemble(const string & outputName);

  void emitDebugSymbols(const string & outputName) const;
  void emitXMLDebugSymbols(const string & outputName) const;
};

#endif	/* ASM_PROGRAM_H */

