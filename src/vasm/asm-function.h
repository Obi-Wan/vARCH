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

typedef list<asm_statement *>         ListOfStmts;
typedef vector<asm_data_statement *>  ListOfDataStmts;

struct asm_function {
  const string name;
  size_t functionOffset;

  YYLTYPE position;

  ListOfStmts stmts;
  ListOfDataStmts uniqueLocals;
  ListOfDataStmts stackLocals;
  ListOfParams parameters;

  TableOfSymbols localSymbols;
  list<ArgLabelRecord *> refs;

  asm_function(const YYLTYPE& pos, const string& _name)
    : name(_name), functionOffset(0), position(pos)
  { }
  asm_function(const YYLTYPE& pos, const char * _name)
    : name(_name), functionOffset(0), position(pos)
  { }

  void finalize();
  void rebuildOffsets();
  bool ensureTempsUsage(const bool & used) const;

  void checkAndAddLabel(asm_statement * stmt);

  void addStmt(asm_statement * stmt) { if (stmt) stmts.push_back(stmt); }
  void addStmts(list<asm_statement *> * stmts);
  void addLocals(list<asm_data_statement *> * locs);
  void addParameter(asm_function_param * p) { if (p) parameters.push_back(p); }

  size_t getInstrSize() const;
  size_t getSharedDataSize() const;

  size_t getStackedDataSize() const;

  size_t getSize() const { return getInstrSize() + getSharedDataSize(); }
};

inline size_t
asm_function::getInstrSize() const
{
  size_t size = 0;
  for(ListOfStmts::const_iterator stmt_it = stmts.begin();
        stmt_it != stmts.end(); stmt_it++)
  {
    const asm_statement * stmt = *stmt_it;
    size += stmt->getSize();
  }
  return size;
}

inline size_t
asm_function::getSharedDataSize() const
{
  size_t size = 0;
  for(ListOfDataStmts::const_iterator stmt_it = uniqueLocals.begin();
      stmt_it != uniqueLocals.end(); stmt_it++)
  {
    size += (*stmt_it)->getSize();
  }
  return size;
}

inline size_t
asm_function::getStackedDataSize() const
{
  size_t size = 0;
  for(ListOfDataStmts::const_iterator stmt_it = stackLocals.begin();
      stmt_it != stackLocals.end(); stmt_it++)
  {
    size += (*stmt_it)->getSize();
  }
  return size;
}

#endif	/* ASM_FUNCTION_H */

