/*
 * IR_LowLevel_Arguments.h
 *
 *  Created on: 29/lug/2011
 *      Author: ben
 */

#ifndef IR_LOWLEVEL_ARGUMENTS_H_
#define IR_LOWLEVEL_ARGUMENTS_H_

#include "IR_LowLevel_Defs.h"

#include "std_istructions.h"

#include "exceptions.h"

#include <string>
#include <vector>

using namespace std;

struct asm_arg {
  enum TypeOfArgument type;
  enum ScaleOfArgument scale;
  enum ModifierOfArgument regModType;
  size_t relOffset;

  YYLTYPE position;

  asm_arg(const YYLTYPE& pos, const TypeOfArgument& _type,
          const ScaleOfArgument & _scale,
          const ModifierOfArgument & _rmt)
    : type(_type), scale(_scale), regModType(_rmt), relOffset(0), position(pos)
  { }
  asm_arg(const YYLTYPE& pos) : relOffset(0), position(pos) { }

  virtual ~asm_arg() { }

  virtual const string toString() const { return ""; }
  virtual const uint32_t getCode() const { return 0; }
  virtual const ObjType getType() const throw() { return ASM_ARG; }

  virtual bool isTemporary() const throw() { return false; }
  virtual bool isReg() const throw() { return false; }
};

struct asm_immediate_arg : asm_arg {
  union {
    enum Registers regNum;
    uint32_t tempUID;
    int32_t val;
    int64_t lval;
    float fval;
  } content;

  bool isTemp;

  int32_t displacement;
  uint32_t index;
  bool isIndexTemp;

  asm_immediate_arg(const YYLTYPE & pos)
    : asm_arg(pos), isTemp(false), displacement(0), index(0)
    , isIndexTemp(false)
  { }
  asm_immediate_arg(const YYLTYPE & pos, const int & _val,
                    const TypeOfArgument & type, const ScaleOfArgument & scale,
                    const ModifierOfArgument & rmt,
                    const bool & _isTemp = false)
    : asm_arg(pos, type, scale, rmt), isTemp(_isTemp), displacement(0)
    , index(0), isIndexTemp(false)
  {
    content.val = _val;
  }
  asm_immediate_arg(const YYLTYPE & pos, const float & _fval)
    : asm_arg(pos, IMMED, BYTE4, REG_NO_ACTION), isTemp(false), displacement(0)
    , index(0), isIndexTemp(false)
  {
    content.fval = _fval;
  }

  const uint32_t getCode() const {
    switch (type) {
      case IMMED:
      case DIRECT:
        return content.tempUID; // Because it is an unsigned
//        return content.val;
      case REG:
      case REG_INDIR:
      case MEM_INDIR:
        return content.regNum + BUILD_PRE_POST_MOD(regModType);
      case DISPLACED:
        return BUILD_BIG_DISPL(displacement)
            + BUILD_FIRST_REG(content.regNum + BUILD_PRE_POST_MOD(regModType));
      case INDEXED:
        return BUILD_INDEX_REG(index + BUILD_PRE_POST_MOD(regModType))
            + BUILD_FIRST_REG(content.regNum);
      case INDX_DISP:
        return BUILD_INDEX_DISPL(displacement)
            + BUILD_INDEX_REG(index + BUILD_PRE_POST_MOD(regModType))
            + BUILD_FIRST_REG(content.regNum);
      default:
        throw WrongArgumentException("getCode() No such kind of argument");
    }
  }
  const ObjType getType() const throw() { return ASM_IMMEDIATE_ARG; }
  const string toString() const { return "(immediate arg)"; }

  virtual bool isTemporary() const throw() { return isTemp; }
  virtual bool isReg() const throw() { return (type != IMMED && type != DIRECT); }
};

struct asm_function_param : asm_arg {
  asm_arg * source, * destination;

  asm_function_param(const YYLTYPE& pos, asm_arg * _source, asm_arg * _dest)
    : asm_arg(pos, REG, BYTE4, REG_NO_ACTION)
    , source(_source), destination(_dest)
  { }
};

typedef vector<asm_function_param *>        ListOfParams;
typedef vector<const asm_function_param *>  ConstListOfParams;

struct asm_label_arg : asm_arg {
  string label;
  uint32_t pointedPosition;

  asm_label_arg(const YYLTYPE& pos, const string& _lab, const TypeOfArgument& _type)
    : asm_arg(pos, _type, BYTE4, REG_NO_ACTION), label(_lab) { }
  asm_label_arg(const YYLTYPE& pos, const char * _lab, const TypeOfArgument& _type)
    : asm_arg(pos, _type, BYTE4, REG_NO_ACTION), label(_lab) { }

  const string toString() const { return string("(label: '") + label + "')"; }
  const uint32_t getCode() const { return pointedPosition; }
  const ObjType getType() const throw() { return ASM_LABEL_ARG; }
};

#endif /* IR_LOWLEVEL_ARGUMENTS_H_ */
