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
#include "asm_helpers.h"

#include <string>
#include <vector>

using namespace std;

struct asm_arg {
  enum TypeOfArgument type;
  int relOffset;
  bool relative;

  YYLTYPE position;

  asm_arg(const YYLTYPE& pos, const TypeOfArgument& _type)
            : type(_type), relOffset(0), relative(false), position(pos) { }
  asm_arg(const YYLTYPE& pos) : relOffset(0), relative(false), position(pos) { }
  virtual const string toString() const { return ""; }
  virtual const int getCode() const { }
  virtual const ObjType getType() const throw() { return ASM_ARG; }

  virtual bool isTemporary() const throw() { return false; }
  virtual bool isReg() const throw() { return false; }
};

struct asm_immediate_arg : asm_arg {
  union {
    enum Registers regNum;
    uint32_t tempUID;
    int val;
    float fval;
  } content;

  bool isTemp;

  asm_immediate_arg(const YYLTYPE& pos) : asm_arg(pos), isTemp(false) { }
  asm_immediate_arg(const YYLTYPE& pos, const int _val,
                    const TypeOfArgument& type, const bool & _isTemp = false)
    : asm_arg(pos, type), isTemp(_isTemp)
  {
    content.val = _val;
  }
  asm_immediate_arg(const YYLTYPE& pos, const float _fval)
    : asm_arg(pos, COST), isTemp(false)
  {
    content.fval = _fval;
  }

  const int getCode() const { return content.val; }
  const ObjType getType() const throw() { return ASM_IMMEDIATE_ARG; }
  const string toString() const { return "(immediate arg)"; }

  virtual bool isTemporary() const throw() { return isTemp; }
  virtual bool isReg() const throw() { return (type != COST && type != ADDR); }
};

struct asm_function_param : asm_arg {
  union {
    enum Registers regNum;
    int address;
  } content;

//  bool isReg;
//
//  asm_function_param(const YYLTYPE& pos, const int & _addr)
//    : asm_arg(pos, ADDR), isReg(false)
//  { content.address = _addr; }
  asm_function_param(const YYLTYPE& pos, const asm_arg * reg)
    : asm_arg(pos, REG) //, isReg(true)
  { content.regNum = ((const asm_immediate_arg *)reg)->content.regNum; }
};

typedef vector<asm_function_param *>        ListOfParams;
typedef vector<const asm_function_param *>  ConstListOfParams;

struct asm_label_arg : asm_arg {
  string label;
  int pointedPosition;

  asm_label_arg(const YYLTYPE& pos, const string& _lab, const TypeOfArgument& _type)
    : asm_arg(pos, _type), label(_lab) { }
  asm_label_arg(const YYLTYPE& pos, const char * _lab, const TypeOfArgument& _type)
    : asm_arg(pos, _type), label(_lab) { }

  const string toString() const { return string("(label: '") + label + "')"; }
  const int getCode() const { return pointedPosition; }
  const ObjType getType() const throw() { return ASM_LABEL_ARG; }
};

#endif /* IR_LOWLEVEL_ARGUMENTS_H_ */
