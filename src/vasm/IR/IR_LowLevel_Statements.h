/*
 * IR_LowLevel_Statements.h
 *
 *  Created on: 29/lug/2011
 *      Author: ben
 */

#ifndef IR_LOWLEVEL_STATEMENTS_H_
#define IR_LOWLEVEL_STATEMENTS_H_

#include "IR_LowLevel_Arguments.h"

#include "masks.h"

struct asm_statement {
  size_t offset;

  YYLTYPE position;

  asm_statement(const YYLTYPE& pos) : offset(0), position(pos) { }
  asm_statement(const YYLTYPE& pos, const int& _offs)
    : offset(_offs), position(pos) { }
  asm_statement(const asm_statement &) = default;

  virtual ~asm_statement() = default;

  virtual bool isInstruction() const throw() { return false; }

  virtual const string toString() const { return ""; }
  virtual size_t getSize() const throw() { return 0; }

  virtual void emitCode(Bloat::iterator & position) { }
  virtual const ObjType getType() const throw() { return ASM_STATEMENT; }

  virtual asm_statement * getCopy() const { return new asm_statement(*this); }
};

//////////////////
// INSTRUCTIONS //
//////////////////

struct asm_instruction_statement : asm_statement {

  const int32_t instruction;

  vector<asm_arg *> args;

  asm_instruction_statement(const YYLTYPE& pos, const int32_t _instr)
    : asm_statement(pos), instruction(_instr)
  { }
  asm_instruction_statement(const asm_instruction_statement & stmt)
    : asm_statement(stmt.position), instruction(stmt.instruction)
  {
    for(asm_arg * const arg : stmt.args)
    {
      this->args.push_back(arg->getCopy());
    }
  }

  ~asm_instruction_statement() {
    for(asm_arg * arg : args) { delete arg; }
  }

  virtual asm_statement * getCopy() const { return new asm_instruction_statement(*this); }

  virtual bool isInstruction() const throw() { return true; }

  asm_instruction_statement * addArg(asm_arg * newArg) {
    args.push_back(newArg);
    newArg->relOffset = args.size();
    return this;
  }

  virtual void checkArgs() const;
  void ensureTempsUsage(const bool & used) const;

  const string toString() const {
    string output = "(instr, ";
    for(asm_arg * arg : args) { output += arg->toString() + " ";  }
    output += " )";
    return output;
  }

  void emitCode(Bloat::iterator & codeIt) {
    int op = instruction;
    for (size_t argNum = 0; argNum < args.size(); argNum++) {
      const asm_arg & arg = *args[argNum];
      op += ARG(argNum, BUILD_ARG(arg.scale, arg.type) );
    }
    const int8_t chunks[4] = DEAL_BWORDS_FROM_SWORD(op);
    for(size_t count = 0; count < 4; count++) {
      *(codeIt++) = chunks[count];
    }

    this->emitArgs(codeIt);
  }

  size_t getSize() const throw() {
    size_t totalSize = 4;
    for(asm_arg * arg : args) { totalSize += arg->getSize(); }
    return totalSize;
  }
  const ObjType getType() const throw() { return ASM_INSTRUCTION_STATEMENT; }

protected:
  void emitArgs(Bloat::iterator & position);
};

/**
 * IR Class holding function calls
 *
 * The first argument is the label of the function, while the other arguments
 * are the parameters
 */
struct asm_function_call : asm_instruction_statement {

  /** Parameters of the called function: don't deallocate them!!! */
  ConstListOfParams parameters;

  asm_function_call(const YYLTYPE& pos, const int _instr)
    : asm_instruction_statement(pos, _instr)
  { }
  asm_function_call(const asm_function_call & stmt)
    : asm_instruction_statement(stmt)
  {
    for(const asm_function_param * const param : stmt.parameters)
    {
      this->parameters.push_back((asm_function_param *)param->getCopy());
    }
  }

  virtual void checkArgs() const;

  void importParameters(const ListOfParams & params);

  void emitCode(Bloat::iterator & codeIt) {
    const asm_arg * arg = args[0];
    const int32_t tempInstr = instruction + ARG_1( BUILD_ARG(arg->scale, arg->type) );

    const int8_t chunks[4] = DEAL_BWORDS_FROM_SWORD(tempInstr);
    for(size_t count = 0; count < 4; count++) {
      *(codeIt++) = chunks[count];
    }

    arg->emitCode(codeIt);
  }

  size_t getSize() const throw() { return (4 + this->args[0]->getSize()); }
  const ObjType getType() const throw() { return ASM_FUNCTION_CALL; }

  virtual asm_statement * getCopy() const { return new asm_function_call(*this); }
};

/**
 * IR Class holding function returns
 *
 * The first and only argument is
 */
struct asm_return_statement : asm_instruction_statement {

  asm_return_statement(const YYLTYPE& pos, const int _instr)
    : asm_instruction_statement(pos, _instr)
  { }
  asm_return_statement(const asm_return_statement & stmt)
    : asm_instruction_statement(stmt)
  { }

  virtual void checkArgs() const;

  void emitCode(Bloat::iterator & codeIt) {
    const int8_t chunks[4] = DEAL_BWORDS_FROM_SWORD(instruction);
    for(size_t count = 0; count < 4; count++) {
      *(codeIt++) = chunks[count];
    }
  }

  size_t getSize() const throw() { return 4; }
  const ObjType getType() const throw() { return ASM_RETURN_STATEMENT; }

  virtual asm_statement * getCopy() const { return new asm_return_statement(*this); }
};

///////////////////////
///////////////////////

struct asm_data_statement : asm_statement {

  bool is_constant;
  bool is_shared;

  asm_data_statement(const YYLTYPE& pos)
    : asm_statement(pos), is_constant(false), is_shared(false)
  { }
  asm_data_statement(const asm_data_statement &) = default;

  const bool & isConst() const throw() { return is_constant; }
  // The idea is to share constants, in order to save moves and allocations
  bool isShared() const throw() { return is_shared || is_constant; }

  const ObjType getType() const throw() { return ASM_DATA_STATEMENT; }

  virtual asm_statement * getCopy() const { return new asm_data_statement(*this); }
};

struct asm_label_statement : asm_data_statement {
  const string label;
  bool is_global;

  asm_label_statement(const YYLTYPE& pos, const string && _label, const bool & _glob = false)
    : asm_data_statement(pos), label(move(_label)), is_global(_glob)
  { }
  asm_label_statement(const YYLTYPE& pos, const string & _label, const bool & _glob = false)
    : asm_data_statement(pos), label(_label), is_global(_glob)
  { }
  asm_label_statement(const asm_label_statement &) = default;

  const string toString() const { return string("(label: '") + label + "')"; }
  const ObjType getType() const throw() { return ASM_LABEL_STATEMENT; }

  const bool & isGlobal() const { return this->is_global; }

  virtual asm_statement * getCopy() const { return new asm_label_statement(*this); }
};

struct asm_data_keyword_statement : asm_data_statement {
  asm_data_keyword_statement(const YYLTYPE& pos) : asm_data_statement(pos) { }
  asm_data_keyword_statement(const asm_data_keyword_statement & ) = default;

  const string toString() const { return "(data_keyword_statement)"; }
  const ObjType getType() const throw() { return ASM_KEYWORD_STATEMENT; }

  virtual asm_statement * getCopy() const { return new asm_data_keyword_statement(*this); }
};

struct asm_int_keyword_statement : asm_data_keyword_statement {
  const int64_t integer;
  const ScaleOfArgument scale;

  asm_int_keyword_statement(const YYLTYPE& pos, const int64_t _integer, const ScaleOfArgument & _scale)
    : asm_data_keyword_statement(pos), integer(_integer), scale(_scale)
  { }
  asm_int_keyword_statement(const asm_int_keyword_statement &) = default;

  const string toString() const { return "(integer_keyword_statement)"; }
  size_t getSize() const throw() { return (1 << scale); }
  const ObjType getType() const throw() { return ASM_INT_KEYWORD_STATEMENT; }

  void emitCode(Bloat::iterator & codeIt);

  virtual asm_statement * getCopy() const { return new asm_int_keyword_statement(*this); }
};

// TODO: compact string code emission
struct asm_string_keyword_statement : asm_data_keyword_statement {
  const string str;

  asm_string_keyword_statement(const YYLTYPE& pos, const string & _str)
    : asm_data_keyword_statement(pos), str(_str)
  { }
  asm_string_keyword_statement(const asm_string_keyword_statement &) = default;

  const string toString() const {
    return  string("(string_keyword_statement: '") + str + "')";
  }
  size_t getSize() const throw() { return str.size(); }
  const ObjType getType() const throw() { return ASM_STRING_KEYWORD_STATEMENT; }

  void emitCode(Bloat::iterator & position) {
    for (size_t i = 0; i < str.size(); i++) {
      *(position++) = (str[i] & BWORD);
    }
  }

  virtual asm_statement * getCopy() const { return new asm_string_keyword_statement(*this); }
};

struct asm_real_keyword_statement : asm_data_keyword_statement {
  float real;

  asm_real_keyword_statement(const YYLTYPE& pos, const float _real)
    : asm_data_keyword_statement(pos), real(_real)
  { }
  asm_real_keyword_statement(const asm_real_keyword_statement &) = default;

  const string toString() const { return  "(real_keyword_statement: )"; }
  size_t getSize() const throw() { return 4; }
  const ObjType getType() const throw() { return ASM_REAL_KEYWORD_STATEMENT; }
  void emitCode(Bloat::iterator & codeIt) {
    const int32_t tempInt = static_cast<int32_t>(real);
    const int8_t number[4] = DEAL_BWORDS_FROM_SWORD(tempInt);
    for(size_t count = 0; count < 4; count++) {
      *(codeIt++) = number[count];
    }
  }

  virtual asm_statement * getCopy() const { return new asm_real_keyword_statement(*this); }
};


#endif /* IR_LOWLEVEL_STATEMENTS_H_ */
