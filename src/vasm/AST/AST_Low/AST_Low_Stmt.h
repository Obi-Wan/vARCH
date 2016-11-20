/*
 * ASTL_Stmt.h
 *
 *  Created on: 20/ott/2011
 *      Author: ben
 */

#ifndef AST_LOW_STMT_H_
#define AST_LOW_STMT_H_

#include "std_istructions.h"
#include "parser_definitions.h"

#include "../../IR/IR_LowLevel_Defs.h"

#include <string>
#include <vector>

using namespace std;

enum ASTL_TypeQualifiers {
  ASTL_stacked,
  ASTL_const,
  ASTL_shared,
};

enum ASTL_Class {
  ASTL_NODE,

  ASTL_ARG,
  ASTL_ARG_LABEL,
  ASTL_ARG_REGISTER,
  ASTL_ARG_SPECIAL_REGISTER,
  ASTL_ARG_NUMBER,

  ASTL_STMT,
  ASTL_STMT_LABEL,
  ASTL_STMT_BASE,
  ASTL_STMT_EXP,

  ASTL_STMT_DECL,
  ASTL_STMT_DECL_NUM,
  ASTL_STMT_DECL_FLOAT,
  ASTL_STMT_DECL_STRING,

  ASTL_PARAM_DECL,
};

class ASTL_Node {
public:
  const YYLTYPE pos;

  ASTL_Node(const YYLTYPE & _pos) : pos(_pos) { }
  virtual ~ASTL_Node() { }

  virtual const string toString() const { return ""; }
  virtual const ASTL_Class getClass() const throw() { return ASTL_NODE; }

  const string getTypeString(const ScaleOfArgument & type) const {
    switch (type) {
      case BYTE8: return "i64_t";
      case BYTE4: return "i32_t";
      case BYTE2: return "i16_t";
      case BYTE1: return "i8_t";
      default:    return "Unknown";
    }
  }
};

////////////////////////////////////////////////////////////////////////////////
/// Arguments Classes
////////////////////////////////////////////////////////////////////////////////

class ASTL_Arg : public ASTL_Node {
public:
  TypeOfArgument kind;
  ScaleOfArgument scale;

  ASTL_Arg(const YYLTYPE & _pos, const TypeOfArgument & _type = IMMED,
      const ScaleOfArgument & _scale = BYTE4)
    : ASTL_Node(_pos), kind(_type), scale(_scale)
  { }
  virtual const string toString() const { return ""; }
  virtual const ASTL_Class getClass() const throw() { return ASTL_ARG; }
};

class ASTL_ArgLabel : public ASTL_Arg {
public:
  const string label;

  ASTL_ArgLabel(const YYLTYPE & _pos, const string && id, const TypeOfArgument & _type,
      const ScaleOfArgument & _scale = BYTE4)
    : ASTL_Arg(_pos, _type, _scale), label(move(id))
  { }

  virtual const string toString() const {
    return string("(Label Arg: ") + label + " : "
        + this->getTypeString(scale) + ")";
  }
  virtual const ASTL_Class getClass() const throw() { return ASTL_ARG_LABEL; }
};

class ASTL_ArgNumber : public ASTL_Arg {
public:
  const string number;

  ASTL_ArgNumber( const YYLTYPE & _pos, const string && _num,
                  const TypeOfArgument & _type = IMMED,
                  const ScaleOfArgument & _scale = BYTE4)
    : ASTL_Arg(_pos, _type, _scale), number(move(_num))
  { }
  virtual const string toString() const {
    return "(Number Arg: " + number + " : "
        + this->getTypeString(scale) + ")";
  }
  virtual const ASTL_Class getClass() const throw() { return ASTL_ARG_NUMBER; }
};

class ASTL_ArgRegister : public ASTL_Arg {
public:
  const string id;

  ModifierOfArgument modif;

  ASTL_ArgNumber * displ;
  ASTL_ArgRegister * index;

  ASTL_ArgRegister(const YYLTYPE & _pos, const string && _id,
      const ScaleOfArgument & _scale = BYTE4)
    : ASTL_Arg(_pos, REG, _scale), id(move(_id)), modif(REG_NO_ACTION)
    , displ(NULL), index(NULL)
  { }
  ~ASTL_ArgRegister();

  virtual const string toString() const {
    return string("(Register Arg: ") + id + " : "
        + this->getTypeString(scale) + ")";
  }
  virtual const ASTL_Class getClass() const throw() { return ASTL_ARG_REGISTER;}

  const string & getRegisterID() const { return id; }
  bool isTemp() const { return (id[0] == 'T'); }
};

class ASTL_ArgSpecialRegister : public ASTL_ArgRegister {
public:
  const Registers regNum;

  ASTL_ArgSpecialRegister(const YYLTYPE & _pos, const Registers & reg,
      const ScaleOfArgument & _scale = BYTE4)
    : ASTL_ArgRegister(_pos, getSpecialRegisterID(reg), _scale), regNum(reg)
  { }
  virtual const string toString() const {
    return string("(Special Register Arg: ") + id + " : "
        + this->getTypeString(scale) + ")";
  }
  virtual const ASTL_Class getClass() const throw() { return ASTL_ARG_SPECIAL_REGISTER;}

  bool isTemp() const { return false; }

  const string getSpecialRegisterID(const Registers & _reg_num) const {
    switch (_reg_num) {
      case PROGRAM_COUNTER:
        return string("PC");
      case STATE_REGISTER:
        return string("SR");
      case STACK_POINTER:
        return string("SP");
      case USER_STACK_POINTER:
        return string("USP");
      default:
        throw WrongArgumentException(string(__FUNCTION__) + ": Unexpected Special Register Number");
    }
  }
};

////////////////////////////////////////////////////////////////////////////////
/// Statements Classes
////////////////////////////////////////////////////////////////////////////////

class ASTL_Stmt : public ASTL_Node {
public:
  ASTL_Stmt(const YYLTYPE & _pos) : ASTL_Node(_pos) { }
  virtual const ASTL_Class getClass() const throw() { return ASTL_STMT; }
  virtual const string toString() const { return ""; }
};

class ASTL_StmtLabel : public ASTL_Stmt {
public:
  const string label;

  string size;
  string num;

  bool constant;
  bool shared;

  ASTL_StmtLabel(const YYLTYPE & _pos, const string && id)
    : ASTL_Stmt(_pos), label(move(id)), size("0"), num("0"), constant(false)
    , shared(false)
  { }
  ASTL_StmtLabel(const ASTL_StmtLabel & old) = default;

  virtual const string toString() const {
    return "(Label stmt: " + label + " ( size: " + size + ", num: " + num + ", "
        + (constant ? "const" : "non-const") + ", "
        + (shared ? "shared" : "non-shared") + " ) )";
  }
  virtual const ASTL_Class getClass() const throw() { return ASTL_STMT_LABEL; }

  void addQualifier(const ASTL_TypeQualifiers & qual) {
    switch (qual) {
      case ASTL_const:
        constant = true;
        break;
      case ASTL_shared:
        shared = true;
        break;
      default:
        // Unknown case. Silently slip away :)
        break;
    }
  }
};

class ASTL_StmtBase : public ASTL_Stmt {
public:
  const int32_t instruction;

  ASTL_StmtBase(const YYLTYPE & _pos, const int32_t & _instr)
    : ASTL_Stmt(_pos), instruction(_instr)
  { }
  virtual const string toString() const {
    return "(Base stmt: " + ISet.getInstr(instruction) + ")";
  }
  virtual const ASTL_Class getClass() const throw() { return ASTL_STMT_BASE; }
};

class ASTL_StmtExp : public ASTL_StmtBase {
public:
  vector<ASTL_Arg *> args;

  ASTL_StmtExp(const YYLTYPE & _pos, const int32_t & _instr)
    : ASTL_StmtBase(_pos, _instr)
  { }

  ~ASTL_StmtExp() {
    for(ASTL_Arg * const arg : args) { delete arg; }
  }

  void addArg(ASTL_Arg * const arg) { args.push_back(arg); }

  virtual const string toString() const {
    string tempOut = "(Base stmt: " + ISet.getInstr(instruction) + " ";
    for(const ASTL_Arg * const arg : args) { arg->toString(); }
    tempOut += " )";
    return tempOut;
  }
  virtual const ASTL_Class getClass() const throw() { return ASTL_STMT_EXP; }
};

class ASTL_VarDecl : public ASTL_Stmt {
public:
  const ScaleOfArgument scale;

  ASTL_VarDecl(const YYLTYPE & _pos, const ScaleOfArgument & _type)
    : ASTL_Stmt(_pos), scale(_type)
  { }

  virtual const string toString() const { return "(Var decl : "
      + this->getTypeString(scale) + " )"; }
  virtual const ASTL_Class getClass() const throw() { return ASTL_STMT_DECL; }
};

class ASTL_VarDeclNumber : public ASTL_VarDecl {
public:
  const ASTL_ArgNumber * number;

  ASTL_VarDeclNumber(const YYLTYPE & _pos, const ScaleOfArgument & _type,
      const ASTL_ArgNumber * num)
    : ASTL_VarDecl(_pos, _type), number(num)
  { }
  ~ASTL_VarDeclNumber();

  virtual const string toString() const { return "(Numberic Var decl : "
      + this->getTypeString(scale) + " )"; }
  virtual const ASTL_Class getClass() const throw() { return ASTL_STMT_DECL_NUM; }
};

class ASTL_VarDeclFloat : public ASTL_VarDecl {
public:
  const ASTL_ArgNumber * number;

  ASTL_VarDeclFloat(const YYLTYPE & _pos, const ASTL_ArgNumber * num)
    : ASTL_VarDecl(_pos, BYTE4), number(num)
  { }
  ~ASTL_VarDeclFloat();

  virtual const string toString() const { return "(Float Var decl : "
      + this->getTypeString(scale) + " )"; }
  virtual const ASTL_Class getClass() const throw() { return ASTL_STMT_DECL_FLOAT; }
};

class ASTL_VarDeclString : public ASTL_VarDecl {
public:
  const string text;

  ASTL_VarDeclString(const YYLTYPE & _pos, const string && _text)
    : ASTL_VarDecl(_pos, BYTE1), text(move(_text))
  { }

  virtual const string toString() const { return "(String Var decl : "
      + this->getTypeString(scale) + " -> \"" + this->text + "\" )"; }
  virtual const ASTL_Class getClass() const throw() { return ASTL_STMT_DECL_STRING; }
};

////////////////////////////////////////////////////////////////////////////////
/// Statements Classes
////////////////////////////////////////////////////////////////////////////////

class ASTL_Param : public ASTL_Node {
public:
  const string srcReg;
  const string destId;

  ASTL_Param(const YYLTYPE & _pos, const string && _src, const string && _dest)
    : ASTL_Node(_pos), srcReg(move(_src)), destId(move(_dest))
  { }

  virtual const string toString() const { return "(Param " + srcReg + " -> " + destId + ")"; }
  virtual const ASTL_Class getClass() const throw() { return ASTL_PARAM_DECL; }
};

#endif /* AST_MID_STMT_H_ */
