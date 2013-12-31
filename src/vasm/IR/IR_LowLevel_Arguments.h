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

#include "BinaryVectors.h"

#include <string>
#include <vector>

using namespace std;

struct asm_arg {
  enum TypeOfArgument type;
  enum ScaleOfArgument scale;

  size_t relOffset;

  YYLTYPE position;

  asm_arg(const YYLTYPE& pos, const TypeOfArgument& _type,
          const ScaleOfArgument & _scale)
    : type(_type), scale(_scale), relOffset(0), position(pos)
  { }
  asm_arg(const YYLTYPE& pos)
    : type(IMMED), scale(BYTE4), relOffset(0), position(pos)
  { }
  asm_arg(const asm_arg &) = default;

  virtual ~asm_arg() = default;

  virtual const string toString() const { return ""; }
  virtual const size_t getSize() const { return 0; }
  virtual void emitCode(Bloat::iterator & codeIt) const { }
  virtual const ObjType getType() const throw() { return ASM_ARG; }

  virtual bool isTemporary() const throw() { return false; }
  virtual bool isReg() const throw() { return false; }

  virtual asm_arg * getCopy() const { return new asm_arg(*this); }
};

struct asm_immediate_arg : asm_arg {
  enum ModifierOfArgument regModType;
  union {
    enum Registers regNum;
    uint32_t tempUID;
    int64_t val;
    float fval;
  } content;

  bool isTemp;

  int32_t displacement;
  uint32_t index;
  bool isIndexTemp;

  asm_immediate_arg(const YYLTYPE & pos)
    : asm_arg(pos), regModType(REG_NO_ACTION), isTemp(false), displacement(0)
    , index(0), isIndexTemp(false)
  {
    content.val = 0;
  }
  asm_immediate_arg(const YYLTYPE & pos, const int64_t & _val,
                    const TypeOfArgument & type, const ScaleOfArgument & scale,
                    const ModifierOfArgument & rmt,
                    const bool & _isTemp = false)
    : asm_arg(pos, type, scale), regModType(rmt), isTemp(_isTemp)
    , displacement(0), index(0), isIndexTemp(false)
  {
    content.val = _val;
  }
  asm_immediate_arg(const YYLTYPE & pos, const float & _fval)
    : asm_arg(pos, IMMED, BYTE4), regModType(REG_NO_ACTION), isTemp(false)
    , displacement(0), index(0), isIndexTemp(false)
  {
    content.fval = _fval;
  }
  asm_immediate_arg(const asm_immediate_arg &) = default;

  void emitCode(Bloat::iterator & codeIt) const;
  const ObjType getType() const throw() { return ASM_IMMEDIATE_ARG; }
  const string toString() const { return "(immediate arg)"; }
  const size_t getSize() const;

  virtual bool isTemporary() const throw() { return isTemp; }
  virtual bool isReg() const throw() { return (type != IMMED && type != DIRECT); }

  virtual asm_arg * getCopy() const { return new asm_immediate_arg(*this); }
};

struct asm_function_param : asm_arg {
  asm_arg * source, * destination;

  asm_function_param(const YYLTYPE& pos, asm_arg * const _source, asm_arg * const _dest)
    : asm_arg(pos, REG, BYTE4)
    , source(_source), destination(_dest)
  { }
  asm_function_param(const asm_function_param &) = default;

  ~asm_function_param() {
    delete this->source;
    delete this->destination;
  }

  virtual asm_arg * getCopy() const { return new asm_function_param(*this); }
};

typedef vector<asm_function_param *>        ListOfParams;
typedef vector<const asm_function_param *>  ConstListOfParams;

struct asm_label_arg : asm_immediate_arg {
  const string label;

  asm_label_arg(const YYLTYPE& pos, const string && _lab
      , const TypeOfArgument& _type, const ScaleOfArgument & _scale = BYTE4)
    : asm_immediate_arg(pos, 0, _type, _scale, REG_NO_ACTION, false), label(move(_lab))
  { }
  asm_label_arg(const YYLTYPE& pos, const string & _lab
      , const TypeOfArgument& _type, const ScaleOfArgument & _scale = BYTE4)
    : asm_immediate_arg(pos, 0, _type, _scale, REG_NO_ACTION, false), label(_lab)
  { }
  asm_label_arg(const asm_immediate_arg & old, const string & _lab)
    : asm_immediate_arg(old), label(_lab)
  { }
  asm_label_arg(const asm_label_arg &) = default;

  const string toString() const { return string("(label: '") + label + "')"; }

  const ObjType getType() const throw() { return ASM_LABEL_ARG; }

  virtual asm_arg * getCopy() const { return new asm_label_arg(*this); }
};

#endif /* IR_LOWLEVEL_ARGUMENTS_H_ */
