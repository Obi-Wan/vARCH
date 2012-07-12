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
#include "BinaryVectors.h"

struct asm_statement {
  size_t offset;

  YYLTYPE position;

  asm_statement(const YYLTYPE& pos) : offset(0), position(pos) { }
  asm_statement(const YYLTYPE& pos, const int& _offs)
    : offset(_offs), position(pos) { }

  virtual ~asm_statement() { }

  virtual bool isInstruction() const throw() { return false; }

  virtual const string toString() const { return ""; }
  virtual size_t getSize() const throw() { return 0; }

  virtual void emitCode(Bloat::iterator & position) { }
  virtual const ObjType getType() const throw() { return ASM_STATEMENT; }
};

//////////////////
// INSTRUCTIONS //
//////////////////

struct asm_instruction_statement : asm_statement {
  int instruction;
  vector<asm_arg *> args;

  asm_instruction_statement(const YYLTYPE& pos, const int _instr)
    : asm_statement(pos), instruction(_instr) { }

  ~asm_instruction_statement() {
    DEALLOC_ELEMS_VECTOR(args, vector<asm_arg *>);
  }

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
    for(size_t argNum = 0; argNum < args.size(); argNum++) {
      output += args[argNum]->toString() + " ";
    }
    output += " )";
    return output;
  }

  void emitCode(Bloat::iterator & codeIt) {
    int op = instruction;
    for (size_t argNum = 0; argNum < args.size(); argNum++) {
      const asm_arg & arg = *args[argNum];
      op += ARG(argNum, BUILD_ARG(arg.scale, arg.type) );
    }
    const int8_t chunks[4] = DEAL_SWORDS_FROM_QWORD(op);
    for(size_t count = 0; count < 4; count++) {
      *(codeIt++) = chunks[count];
    }

    this->emitArgs(codeIt);
  }

  size_t getSize() const throw() {
    size_t totalSize = 4;
    for(size_t argNum = 0; argNum < args.size(); argNum++) {
      totalSize += args[argNum]->getSize();
    }
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

  virtual void checkArgs() const;

  void importParameters(const ListOfParams & params);

  void emitCode(Bloat::iterator & codeIt) {
    const asm_arg * arg = args[0];
    const int32_t tempInstr = instruction + ARG_1( BUILD_ARG(arg->scale, arg->type) );

    const int8_t chunks[4] = DEAL_SWORDS_FROM_QWORD(tempInstr);
    for(size_t count = 0; count < 4; count++) {
      *(codeIt++) = chunks[count];
    }

    arg->emitCode(codeIt);
  }

  size_t getSize() const throw() { return (4 + this->args[0]->getSize()); }
  const ObjType getType() const throw() { return ASM_FUNCTION_CALL; }
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

  virtual void checkArgs() const;

  void emitCode(Bloat::iterator & codeIt) {
    const int8_t chunks[4] = DEAL_SWORDS_FROM_QWORD(instruction);
    for(size_t count = 0; count < 4; count++) {
      *(codeIt++) = chunks[count];
    }
  }

  size_t getSize() const throw() { return 4; }
  const ObjType getType() const throw() { return ASM_RETURN_STATEMENT; }
};

///////////////////////
///////////////////////

struct asm_data_statement : asm_statement {

  bool is_constant;
  bool is_shared;

  asm_data_statement(const YYLTYPE& pos)
    : asm_statement(pos), is_constant(false), is_shared(false) { }

  const bool & isConst() const throw() { return is_constant; }
  // The idea is to share constants, in order to save moves and allocations
  bool isShared() const throw() { return is_shared || is_constant; }

  const ObjType getType() const throw() { return ASM_DATA_STATEMENT; }
};

struct asm_label_statement : asm_data_statement {
  string label;
  bool is_global;

  asm_label_statement(const YYLTYPE& pos, const char * _label, const bool & _glob = false)
    : asm_data_statement(pos), label(_label), is_global(_glob)
  { }
  asm_label_statement(const YYLTYPE& pos, const string& _label, const bool & _glob = false)
    : asm_data_statement(pos), label(_label), is_global(_glob)
  { }

  const string toString() const { return string("(label: '") + label + "')"; }
  const ObjType getType() const throw() { return ASM_LABEL_STATEMENT; }

  const bool & isGlobal() const { return this->is_global; }
};

struct asm_data_keyword_statement : asm_data_statement {
  asm_data_keyword_statement(const YYLTYPE& pos) : asm_data_statement(pos) { }
  const string toString() const { return "(data_keyword_statement)"; }
  const ObjType getType() const throw() { return ASM_KEYWORD_STATEMENT; }
};

struct asm_int_keyword_statement : asm_data_keyword_statement {
  int integer;

  asm_int_keyword_statement(const YYLTYPE& pos, const int _integer)
    : asm_data_keyword_statement(pos), integer(_integer) { }

  const string toString() const { return "(integer_keyword_statement)"; }
  size_t getSize() const throw() { return 4; }
  const ObjType getType() const throw() { return ASM_INT_KEYWORD_STATEMENT; }

  void emitCode(Bloat::iterator & codeIt) {
    const int8_t number[4] = DEAL_SWORDS_FROM_QWORD(integer);
    for(size_t count = 0; count < 4; count++) {
      *(codeIt++) = number[count];
    }
  }
};

struct asm_long_keyword_statement : asm_data_keyword_statement {
  long int longInteger;

  asm_long_keyword_statement(const YYLTYPE& pos, const long int _long)
    : asm_data_keyword_statement(pos), longInteger(_long) { }

  const string toString() const { return "(long_integer_keyword_statement)"; }
  size_t getSize() const throw() { return 8; }
  const ObjType getType() const throw() { return ASM_LONG_KEYWORD_STATEMENT; }

  void emitCode(Bloat::iterator & position) {
    const int8_t number[8] = DEAL_SWORDS_FROM_OWORD(longInteger);
    for(size_t count = 0; count < 8; count++) {
      *(position++) = number[count];
    }
  }
};

struct asm_char_keyword_statement : asm_data_keyword_statement {
  char character;

  asm_char_keyword_statement(const YYLTYPE& pos, const char * _char)
    : asm_data_keyword_statement(pos), character(*(_char)) { }

  const string toString() const { return "(char_keyword_statement)"; }
  size_t getSize() const throw() { return 1; }
  const ObjType getType() const throw() { return ASM_CHAR_KEYWORD_STATEMENT; }

  void emitCode(Bloat::iterator & position) {
    *(position++) = (character & SWORD);
  }
};

struct asm_string_keyword_statement : asm_data_keyword_statement {
  string str;

  asm_string_keyword_statement(const YYLTYPE& pos, const string& _str)
    : asm_data_keyword_statement(pos), str(_str) { }

  const string toString() const {
    return  string("(string_keyword_statement: '") + str + "')";
  }
  size_t getSize() const throw() { return str.size(); }
  const ObjType getType() const throw() { return ASM_STRING_KEYWORD_STATEMENT; }

  void emitCode(Bloat::iterator & position) {
    for (size_t i = 0; i < str.size(); i++) {
      *(position++) = (str[i] & SWORD);
    }
  }
};

struct asm_real_keyword_statement : asm_data_keyword_statement {
  float real;

  asm_real_keyword_statement(const YYLTYPE& pos, const float _real)
    : asm_data_keyword_statement(pos), real(_real) { }

  const string toString() const { return  "(real_keyword_statement: )"; }
  size_t getSize() const throw() { return 4; }
  const ObjType getType() const throw() { return ASM_REAL_KEYWORD_STATEMENT; }
  void emitCode(Bloat::iterator & codeIt) {
    const int32_t tempInt = static_cast<int32_t>(real);
    const int8_t number[4] = DEAL_SWORDS_FROM_QWORD(tempInt);
    for(size_t count = 0; count < 4; count++) {
      *(codeIt++) = number[count];
    }
  }
};


#endif /* IR_LOWLEVEL_STATEMENTS_H_ */
