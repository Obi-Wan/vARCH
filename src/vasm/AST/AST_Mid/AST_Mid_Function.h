/*
 * AST_Mid_Function.h
 *
 *  Created on: 21/ott/2011
 *      Author: ben
 */

#ifndef AST_MID_FUNCTION_H_
#define AST_MID_FUNCTION_H_

#include "AST_Mid_Stmt.h"

#include <map>
#include <list>

class EnvSymbols {

  struct SymbolRecord {
    ASTM_BuiltinTypes type;
    YYLTYPE pos;
  };

  typedef std::map<std::string, SymbolRecord> MapOfSymbols;

public:
  void put(const std::string & name, const ASTM_BuiltinTypes & type,
      const YYLTYPE & pos);
  const ASTM_BuiltinTypes & getType(const std::string & name) const;
};

class ASTM_FunctionProto : public ASTM_Node {
public:
  const ASTM_BuiltinTypes retType;
  const std::string name;

  std::vector<ASTM_Param *> params;

  ASTM_FunctionProto(const YYLTYPE & _pos,
      const ASTM_BuiltinTypes & _ret_type, const std::string & _name,
      std::list<ASTM_Param *> * paramsList)
    : ASTM_Node(_pos), retType(_ret_type), name(_name)
  {
    params.insert(params.end(), paramsList->begin(), paramsList->end());
    delete paramsList;
  }
  ~ASTM_FunctionProto() {
    for(size_t num = 0; num < params.size(); num++) { delete params[num]; }
  }
};

class ASTM_FunctionDef : public ASTM_FunctionProto {
public:
  EnvSymbols symbols;

  std::vector<ASTM_Stmt *> stmts;

  ASTM_FunctionDef(const YYLTYPE & _pos,
      const ASTM_BuiltinTypes & _ret_type, const std::string & _name,
      std::list<ASTM_Param *> * paramsList, std::list<ASTM_Stmt *> * stmtsList)
    : ASTM_FunctionProto(_pos, _ret_type, _name, paramsList)
  {
    stmts.insert(stmts.end(), stmtsList->begin(), stmtsList->end());
    delete stmtsList;
  }
  ~ASTM_FunctionDef() {
    for(size_t num = 0; num < stmts.size(); num++) { delete stmts[num]; }
  }
};

#endif /* AST_MID_FUNCTION_H_ */
