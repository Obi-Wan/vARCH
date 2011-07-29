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
#include "../FileHandler.h"

struct asm_statement {
  size_t offset;

  YYLTYPE position;

  asm_statement(const YYLTYPE& pos) : offset(0), position(pos) { }
  asm_statement(const YYLTYPE& pos, const int& _offs)
    : offset(_offs), position(pos) { }

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

  asm_instruction_statement * addArg(asm_arg * newArg) {
    args.push_back(newArg);
    newArg->relOffset = args.size();
    return this;
  }

  void checkArgs() const;
  void ensureTempsUsage(const bool & used) const;

  const string toString() const {
    string output = "(instr, ";
    for(size_t argNum = 0; argNum < args.size(); argNum++) {
      output += args[argNum]->toString() + " ";
    }
    output += " )";
    return output;
  }

  void emitCode(Bloat::iterator & position) {
    int op = instruction;
    for (size_t argNum = 0; argNum < args.size(); argNum++) {
      const asm_arg & arg = *args[argNum];
      op += ARG(argNum, (arg.type + (arg.relative ? RELATIVE_ARG : 0)) );
    }
    *(position++) = op;

    for (size_t argNum = 0; argNum < args.size(); argNum++) {
      *(position++) = args[argNum]->getCode();
    }
  }

  size_t getSize() const throw() { return 1 + args.size(); }
  const ObjType getType() const throw() { return ASM_INSTRUCTION_STATEMENT; }
};

///////////////////////
///////////////////////

struct asm_data_statement : asm_statement {
  asm_data_statement(const YYLTYPE& pos) : asm_statement(pos) { }
  const ObjType getType() const throw() { return ASM_DATA_STATEMENT; }
};

struct asm_label_statement : asm_data_statement {
  string label;

  asm_label_statement(const YYLTYPE& pos, const char * _label)
    : asm_data_statement(pos), label(_label) { }
  asm_label_statement(const YYLTYPE& pos, const string& _label)
    : asm_data_statement(pos), label(_label) { }

  const string toString() const { return string("(label: '") + label + "')"; }
  const ObjType getType() const throw() { return ASM_LABEL_STATEMENT; }
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
  size_t getSize() const throw() { return 1; }
  const ObjType getType() const throw() { return ASM_INT_KEYWORD_STATEMENT; }

  void emitCode(Bloat::iterator & position) {
    *(position++) = integer;
  }
};

struct asm_long_keyword_statement : asm_data_keyword_statement {
  long int longInteger;

  asm_long_keyword_statement(const YYLTYPE& pos, const long int _long)
    : asm_data_keyword_statement(pos), longInteger(_long) { }

  const string toString() const { return "(long_integer_keyword_statement)"; }
  size_t getSize() const throw() { return 2; }
  const ObjType getType() const throw() { return ASM_LONG_KEYWORD_STATEMENT; }

  void emitCode(Bloat::iterator & position) {
    *(position++) = EXTRACT_HIGHER_SWORD_FROM_DWORD(longInteger);
    *(position++) = EXTRACT_LOWER__SWORD_FROM_DWORD(longInteger);
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
    *(position++) = (character & 0xff );
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
      *(position++) = (str[i] & 0xff );
    }
  }
};

struct asm_real_keyword_statement : asm_data_keyword_statement {
  float real;

  asm_real_keyword_statement(const YYLTYPE& pos, const float _real)
    : asm_data_keyword_statement(pos), real(_real) { }

  const string toString() const { return  "(real_keyword_statement: )"; }
  size_t getSize() const throw() { return 1; }
  const ObjType getType() const throw() { return ASM_REAL_KEYWORD_STATEMENT; }
  void emitCode(Bloat::iterator & position) {
    *(position++) = static_cast<int>(real);
  }
};


#endif /* IR_LOWLEVEL_STATEMENTS_H_ */
