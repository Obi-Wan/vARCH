/*
 * AST_Low_Function.h
 *
 *  Created on: 21/ott/2011
 *      Author: ben
 */

#ifndef AST_LOW_FUNCTION_H_
#define AST_LOW_FUNCTION_H_

#include "AST_Low_Stmt.h"

#include <map>
#include <list>

using namespace std;

class EnvSymbols {

  struct SymbolRecord {
    ScaleOfArgument type;
    YYLTYPE pos;
  };

  typedef map<string, SymbolRecord> MapOfSymbols;

public:
  void put(const string & name, const ScaleOfArgument & type,
      const YYLTYPE & pos);
  const ScaleOfArgument & getType(const string & name) const;
};

class ASTL_FunctionProto : public ASTL_Node {
public:
  const string name;

  vector<ASTL_Param *> params;

  ASTL_FunctionProto(const YYLTYPE & _pos, const string & _name)
    : ASTL_Node(_pos), name(_name)
  { }

  void addParameter(ASTL_Param * param) { params.push_back(param); }

  ~ASTL_FunctionProto() {
    for(size_t num = 0; num < params.size(); num++) { delete params[num]; }
  }
};

class ASTL_FunctionDef : public ASTL_FunctionProto {
public:
  EnvSymbols symbols;

  vector<ASTL_Stmt *> locals;
  vector<ASTL_Stmt *> stmts;

  ASTL_FunctionDef(const YYLTYPE & _pos, const string & _name)
    : ASTL_FunctionProto(_pos, _name)
  { }

  ~ASTL_FunctionDef() {
    for(size_t num = 0; num < stmts.size(); num++) { delete stmts[num]; }
  }

  void addStmt(ASTL_Stmt * stmt) { stmts.push_back(stmt); }

  void addLocals(list<ASTL_Stmt *> * stmtsList) {
    locals.insert(stmts.end(), stmtsList->begin(), stmtsList->end());
  }

  void finalize() { }
};

#endif /* AST_LOW_FUNCTION_H_ */
