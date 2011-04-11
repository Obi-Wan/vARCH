/* 
 * File:   asm_function.h
 * Author: ben
 *
 * Created on 5 agosto 2010, 21.13
 */

#ifndef ASM_FUNCTION_H
#define	ASM_FUNCTION_H

#include "asm-classes.h"

struct asm_function {
  const string name;
  int tempLocalOffset;
  int funcOffset;

  YYLTYPE position;

  vector<asm_statement *> stmts;
  vector<asm_data_statement *> locals;

  TableOfSymbols localSymbols;
  list<argLabelRecord *> refs;

  asm_function(const YYLTYPE& pos, const string& _name,
               list<asm_statement *> * _stmts,
               list<asm_data_statement *> * _locals);
  asm_function(const YYLTYPE& pos, const char * _name,
               list<asm_statement *> * _stmts,
               list<asm_data_statement *> * _locals);

  void init(list<asm_statement *> *_stmts, list<asm_data_statement *> *_locals);
  bool checkInstructions() const;
  void checkLabel(asm_statement * stmt);

  int getInstrSize() const {
    int size = 0;
    for(size_t index = 0; index < stmts.size(); index++) {
      size += stmts[index]->getSize();
    }
    return size;
  }
  int getDataSize() const {
    int size = 0;
    for(size_t index = 0; index < locals.size(); index++) {
      size += locals[index]->getSize();
    }
    return size;
  }
  int getSize() const { return getInstrSize() + getDataSize(); }
};

#endif	/* ASM_FUNCTION_H */

