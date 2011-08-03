/* 
 * File:   asm_function.h
 * Author: ben
 *
 * Created on 5 agosto 2010, 21.13
 */

#ifndef ASM_FUNCTION_H
#define	ASM_FUNCTION_H

#include "IR/IR_LowLevel_Statements.h"
#include "algorithms/Labels.h"

#include <deque>
#include <list>

using namespace std;

typedef list<asm_statement *> ListOfStmts;

struct asm_function {
  const string name;
  int tempLocalOffset;
  int funcOffset;

  YYLTYPE position;

  ListOfStmts stmts;
  vector<asm_data_statement *> locals;
  ListOfParams parameters;

  TableOfSymbols localSymbols;
  list<ArgLabelRecord *> refs;

  asm_function(const YYLTYPE& pos, const string& _name)
    : name(_name), tempLocalOffset(0), funcOffset(0), position(pos)
  { }
  asm_function(const YYLTYPE& pos, const char * _name)
    : name(_name), tempLocalOffset(0), funcOffset(0), position(pos)
  { }

  void finalize();
  void rebuildOffsets();
  bool ensureTempsUsage(const bool & used) const;

  void checkLabel(asm_statement * stmt);

  void addStmt(asm_statement * stmt) { if (stmt) stmts.push_back(stmt); }
  void addLocals(list<asm_data_statement *> * locs)
  {
    locals.insert(locals.begin(), locs->begin(), locs->end());
  }
  void addParameter(asm_function_param * p) { if (p) parameters.push_back(p); }

  size_t getInstrSize() const {
    size_t size = 0;
    for(ListOfStmts::const_iterator stmt_it = stmts.begin();
          stmt_it != stmts.end(); stmt_it++)
    {
      const asm_statement * stmt = *stmt_it;
      size += stmt->getSize();
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

