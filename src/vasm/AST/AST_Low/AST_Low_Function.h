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

class EnvSymbols {

  struct SymbolRecord {
    ScaleOfArgument type;
    YYLTYPE pos;
    size_t size;
  };

  typedef std::map<std::string, SymbolRecord> MapOfSymbols;

public:
  void put(const std::string & name, const YYLTYPE & pos,
      const ScaleOfArgument & type, const size_t & size);
  const ScaleOfArgument & getType(const std::string & name) const;
  const size_t & getSize(const std::string & name) const;
};

class ASTL_FunctionProto : public ASTL_Node {
public:
  const std::string name;

  std::vector<ASTL_Param *> params;

  ASTL_FunctionProto(const YYLTYPE & _pos, const std::string && _name)
    : ASTL_Node(_pos), name(move(_name))
  { }
  ASTL_FunctionProto(const YYLTYPE & _pos, const std::string & _name)
    : ASTL_Node(_pos), name(_name)
  { }
  ASTL_FunctionProto(const ASTL_FunctionProto & old)
    : ASTL_Node(old.pos), name(old.name)
  { this->copyParameters(old.params); }

  void addParameter(ASTL_Param * const param) { params.push_back(param); }
  void copyParameters(const std::vector<ASTL_Param *> & _params);

  ~ASTL_FunctionProto() {
    for(ASTL_Param * param : params) { delete param; }
  }

  void finalize();
  virtual void printFunction();
};

class ASTL_FunctionDef : public ASTL_FunctionProto {
public:
  EnvSymbols symbols;

  std::vector<ASTL_Stmt *> locals;
  std::vector<ASTL_Stmt *> stmts;

  ASTL_FunctionDef(const YYLTYPE & _pos, const std::string && _name)
    : ASTL_FunctionProto(_pos, move(_name))
  { }
  ASTL_FunctionDef(const ASTL_FunctionProto & func)
    : ASTL_FunctionProto(func)
  { }

  ~ASTL_FunctionDef();

  void addStmt(ASTL_Stmt * const stmt) { stmts.push_back(stmt); }

  void addLocals(std::list<ASTL_Stmt *> * const stmtsList) {
    locals.insert(stmts.end(), stmtsList->begin(), stmtsList->end());
  }

  virtual void printFunction();
};

#endif /* AST_LOW_FUNCTION_H_ */
