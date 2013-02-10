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

  asm_function(const YYLTYPE& pos, const string & _name)
    : name(_name), functionOffset(0), position(pos)
  { }
  asm_function(const YYLTYPE& pos, const string && _name)
    : name(move(_name)), functionOffset(0), position(pos)
  { }
  ~asm_function();

  void finalize();

  void checkAndAddLabel(asm_statement * stmt);

  void addStmt(asm_statement * stmt) { if (stmt) stmts.push_back(stmt); }
  void addStmts(list<asm_statement *> && stmts);
  void addLocals(list<asm_data_statement *> && locs);
  void addParameter(asm_function_param * p) { if (p) parameters.push_back(p); }

  size_t getInstrSize() const;
  size_t getSharedDataSize() const;

  size_t getStackedDataSize() const;

  size_t getPaddingSize() const;

  size_t getSize() const {
    return getInstrSize() + getSharedDataSize() + getPaddingSize();
  }
};

inline size_t
asm_function::getInstrSize() const
{
  size_t size = 0;
  for(const asm_statement * stmt : stmts) { size += stmt->getSize(); }
  return size;
}

inline size_t
asm_function::getSharedDataSize() const
{
  size_t size = 0;
  for(const asm_data_statement * stmt : uniqueLocals) { size += stmt->getSize(); }
  return size;
}

inline size_t
asm_function::getStackedDataSize() const
{
  size_t size = 0;
  for(const asm_data_statement * stmt : stackLocals)
  {
    size += stmt->getSize();
    /* Let's re align to 32bits, because this is what the PUSH instruction is
     * going to do */
    const size_t rest = size % 4;
    size += (4 - rest) * (rest != 0);
  }
  return size;
}

inline size_t
asm_function::getPaddingSize() const
{
  const size_t rest = ((getInstrSize() + getSharedDataSize()) % 4);
  return (4 - rest) * (rest != 0);
}

#endif	/* ASM_FUNCTION_H */

