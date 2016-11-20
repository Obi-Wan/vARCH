/*
 * ASTM_Stmt.h
 *
 *  Created on: 20/ott/2011
 *      Author: ben
 */

#ifndef AST_MID_STMT_H_
#define AST_MID_STMT_H_

#include "std_istructions.h"

#include "../../IR/IR_LowLevel_Defs.h"

#include <string>
#include <vector>

enum ASTM_TypeQualifiers {
  ASTM_stacked,
  ASTM_const,
  ASTM_shared,
};

enum ASTM_BuiltinTypes {
  ASTM_none = 0,
  ASTM_auto,

  ASTM_i8,
  ASTM_i16,
  ASTM_i32,
  ASTM_i64,

  ASTM_u8,
  ASTM_u16,
  ASTM_u32,
  ASTM_u64,

  ASTM_f32,
  ASTM_f64,

  ASTM_void,

  ASTM_ptr_i8,
  ASTM_ptr_i16,
  ASTM_ptr_i32,
  ASTM_ptr_i64,

  ASTM_ptr_u8,
  ASTM_ptr_u16,
  ASTM_ptr_u32,
  ASTM_ptr_u64,

  ASTM_ptr_f32,
  ASTM_ptr_f64,

  ASTM_ptr_void,
};

enum ASTM_Class {
  ASTM_NODE,

  ASTM_ARG,
  ASTM_ARG_LABEL,
  ASTM_ARG_REGISTER,
  ASTM_ARG_NUMBER,
};

class ASTM_Node {
public:
  const YYLTYPE pos;

  ASTM_Node(const YYLTYPE & _pos) : pos(_pos) { }
  virtual ~ASTM_Node() { }

  virtual const std::string toString() const { return ""; }
  virtual const ASTM_Class getClass() const throw() { return ASTM_NODE; }
};

////////////////////////////////////////////////////////////////////////////////
/// Arguments Classes
////////////////////////////////////////////////////////////////////////////////

class ASTM_Arg : public ASTM_Node {
public:
  const ASTM_BuiltinTypes type;

  ASTM_Arg(const YYLTYPE & _pos,
      const ASTM_BuiltinTypes & _type = ASTM_auto)
    : ASTM_Node(_pos), type(_type)
  { }
  virtual const ASTM_Class getClass() const throw() { return ASTM_ARG; }
};

class ASTM_ArgLabel : public ASTM_Arg {
public:
  const std::string label;
  const bool relative;

  ASTM_ArgLabel(const YYLTYPE & _pos, const std::string & id,
      const bool & _rel = false)
    : ASTM_Arg(_pos, ASTM_ptr_void), label(id), relative(_rel)
  { }

  virtual const std::string toString() const {
    return "";
  }
  virtual const ASTM_Class getClass() const throw() { return ASTM_ARG_LABEL; }
};

class ASTM_ArgRegister : public ASTM_Arg {
public:
  const std::string id;
  const TypeOfArgument kind;
  const bool relative;

  ASTM_ArgRegister(const YYLTYPE & _pos, const std::string & _id,
      const ASTM_BuiltinTypes & _type, const TypeOfArgument & _kind,
      const bool & _rel = false)
    : ASTM_Arg(_pos, _type), id(_id), kind(_kind), relative(_rel)
  { }
  virtual const ASTM_Class getClass() const throw() { return ASTM_ARG_REGISTER;}
};

class ASTM_ArgNumber : public ASTM_Arg {
public:
  const std::string number;

  ASTM_ArgNumber(const YYLTYPE & _pos, const std::string & _num,
      const ASTM_BuiltinTypes & _type)
    : ASTM_Arg(_pos, _type), number(_num)
  { }
  virtual const ASTM_Class getClass() const throw() { return ASTM_ARG_NUMBER; }
};

////////////////////////////////////////////////////////////////////////////////
/// Statements Classes
////////////////////////////////////////////////////////////////////////////////

class ASTM_Stmt : public ASTM_Node {
public:
  ASTM_Stmt(const YYLTYPE & _pos) : ASTM_Node(_pos) { }
};

class ASTM_StmtLabel : public ASTM_Stmt {
public:
  const std::string label;

  ASTM_StmtLabel(const YYLTYPE & _pos, const std::string & id)
    : ASTM_Stmt(_pos), label(id)
  { }
};

class ASTM_StmtBase : public ASTM_Stmt {
public:
  const int32_t instruction;
  const ASTM_BuiltinTypes retType;

  ASTM_StmtBase(const YYLTYPE & _pos, const int32_t & _instr,
      const ASTM_BuiltinTypes & _ret_type = ASTM_auto)
    : ASTM_Stmt(_pos), instruction(_instr), retType(_ret_type)
  { }
};

class ASTM_StmtExp : public ASTM_StmtBase {
public:
  std::vector<ASTM_Arg *> args;

  ASTM_StmtExp(const YYLTYPE & _pos, const int32_t & _instr,
      const ASTM_BuiltinTypes & _ret_type = ASTM_auto)
    : ASTM_StmtBase(_pos, _instr, _ret_type)
  { }

  ~ASTM_StmtExp() {
    for(size_t num = 0; num < args.size(); num++) { delete args[num]; }
  }

  void addArg(ASTM_Arg * const arg) { args.push_back(arg); }
};

class ASTM_StmtSpecialExp : public ASTM_StmtBase {
public:
  ASTM_Arg * const arg;

  ASTM_StmtSpecialExp(const YYLTYPE & _pos, const int32_t & _instr,
      const ASTM_BuiltinTypes & _ret_type, ASTM_Arg * const _arg)
    : ASTM_StmtBase(_pos, _instr, _ret_type), arg(_arg)
  { }
  ~ASTM_StmtSpecialExp() { delete arg; }
};

class ASTM_StmtAssign : public ASTM_Stmt {
public:
  const std::string assignedId;

  ASTM_Node * const expr;

  ASTM_StmtAssign(const YYLTYPE & _pos, const std::string & _id,
      ASTM_Node * const _expr)
    : ASTM_Stmt(_pos), assignedId(_id), expr(_expr)
  { }
  ~ASTM_StmtAssign() { delete expr; }
};

class ASTM_VarDecl : public ASTM_Stmt {
public:
  const bool constant;
  const bool shared;

  const ASTM_BuiltinTypes type;

  ASTM_StmtAssign * const assign;

  ASTM_VarDecl(const YYLTYPE & _pos, const bool & _const,
      const bool & _shared, const ASTM_BuiltinTypes & _type,
      ASTM_StmtAssign * const _stmt)
    : ASTM_Stmt(_pos), constant(_const), shared(_shared), type(_type)
    , assign(_stmt)
  { }
};

//class ASTM_FieldDecl : public ASTM_Stmt {
//public:
//
//};

////////////////////////////////////////////////////////////////////////////////
/// Statements Classes
////////////////////////////////////////////////////////////////////////////////

class ASTM_Param : public ASTM_Node {
public:
  const ASTM_BuiltinTypes type;
  const std::string id;

  ASTM_Param(const YYLTYPE & _pos, const ASTM_BuiltinTypes & _type,
      const std::string & _id)
    : ASTM_Node(_pos), type(_type), id(_id)
  { }

  virtual const std::string toString() const { return "Name: " + id; }
};

#endif /* AST_MID_STMT_H_ */
