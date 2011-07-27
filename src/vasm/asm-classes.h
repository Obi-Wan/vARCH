
#ifndef ASM_PARSE_DEF_H
#define ASM_PARSE_DEF_H

#include <vector>
#include <list>
#include <deque>
#include <string>
#include <cstdio>
#include <cstring>
#include <map>

#include "asm_helpers.h"
#include "std_istructions.h"
#include "macros.h"
#include "masks.h"
#include "exceptions.h"
#include "../FileHandler.h"

using namespace std;

class IncludesNode;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  IncludesNode * fileNode;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

# define YYLLOC_DEFAULT(Current, Rhs, N)\
    do\
      if (YYID (N))\
  {\
    (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;  \
    (Current).first_column = YYRHSLOC (Rhs, 1).first_column;\
    (Current).last_line    = YYRHSLOC (Rhs, N).last_line;   \
    (Current).last_column  = YYRHSLOC (Rhs, N).last_column; \
    (Current).fileNode     = YYRHSLOC (Rhs, N).fileNode;    \
  }\
      else\
  {\
    (Current).first_line   = (Current).last_line   =    \
      YYRHSLOC (Rhs, 0).last_line;        \
    (Current).first_column = (Current).last_column =    \
      YYRHSLOC (Rhs, 0).last_column;      \
    (Current).fileNode     = YYRHSLOC (Rhs, 0).fileNode;\
  }\
    while (YYID (0))

enum ObjType {
  ASM_ARG,
  ASM_FUNCTION_ARG,
  ASM_IMMEDIATE_ARG,
  ASM_LABEL_ARG,
  ASM_STATEMENT,
  ASM_INSTRUCTION_STATEMENT,
  ASM_DATA_STATEMENT,
  ASM_LABEL_STATEMENT,
  ASM_KEYWORD_STATEMENT,
  ASM_INT_KEYWORD_STATEMENT,
  ASM_LONG_KEYWORD_STATEMENT,
  ASM_REAL_KEYWORD_STATEMENT,
  ASM_CHAR_KEYWORD_STATEMENT,
  ASM_STRING_KEYWORD_STATEMENT,
};

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
  virtual const ObjType getType() const { return ASM_ARG; }
};

struct asm_immediate_arg : asm_arg {
  union {
    enum Registers regNum;
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
  const ObjType getType() const { return ASM_IMMEDIATE_ARG; }
  const string toString() const { return "(immediate arg)"; }
};

struct asm_function_param : asm_arg {
  union {
    enum Registers regNum;
    int address;
  } content;

  bool isReg;

  asm_function_param(const YYLTYPE& pos, const int & _addr)
    : asm_arg(pos, ADDR), isReg(false)
  { content.address = _addr; }
  asm_function_param(const YYLTYPE& pos, const asm_arg * reg)
    : asm_arg(pos, REG), isReg(true)
  { content.regNum = ((const asm_immediate_arg *)reg)->content.regNum; }
};

struct asm_label_arg : asm_arg {
  string label;
  int pointedPosition;

  asm_label_arg(const YYLTYPE& pos, const string& _lab, const TypeOfArgument& _type)
    : asm_arg(pos, _type), label(_lab) { }
  asm_label_arg(const YYLTYPE& pos, const char * _lab, const TypeOfArgument& _type)
    : asm_arg(pos, _type), label(_lab) { }

  const string toString() const { return string("(label: '") + label + "')"; }
  const int getCode() const { return pointedPosition; }
  const ObjType getType() const { return ASM_LABEL_ARG; }
};

struct asm_statement {
  int offset;

  YYLTYPE position;

  asm_statement(const YYLTYPE& pos) : offset(0), position(pos) { }
  asm_statement(const YYLTYPE& pos, const int& _offs)
    : offset(_offs), position(pos) { }

  virtual const string toString() const { return ""; }
  virtual int getSize() const { return 0; }

  virtual void emitCode(Bloat::iterator & position) { }
  virtual const ObjType getType() const { return ASM_STATEMENT; }
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

  int getSize() const { return 1 + args.size(); }
  const ObjType getType() const { return ASM_INSTRUCTION_STATEMENT; }
};

///////////////////////
///////////////////////

struct asm_data_statement : asm_statement {
  asm_data_statement(const YYLTYPE& pos) : asm_statement(pos) { }
  const ObjType getType() const { return ASM_DATA_STATEMENT; }
};

struct asm_label_statement : asm_data_statement {
  string label;

  asm_label_statement(const YYLTYPE& pos, const char * _label)
    : asm_data_statement(pos), label(_label) { }
  asm_label_statement(const YYLTYPE& pos, const string& _label)
    : asm_data_statement(pos), label(_label) { }

  const string toString() const { return string("(label: '") + label + "')"; }
  const ObjType getType() const { return ASM_LABEL_STATEMENT; }
};

struct asm_data_keyword_statement : asm_data_statement {
  asm_data_keyword_statement(const YYLTYPE& pos) : asm_data_statement(pos) { }
  const string toString() const { return "(data_keyword_statement)"; }
  const ObjType getType() const { return ASM_KEYWORD_STATEMENT; }
};

struct asm_int_keyword_statement : asm_data_keyword_statement {
  int integer;

  asm_int_keyword_statement(const YYLTYPE& pos, const int _integer)
    : asm_data_keyword_statement(pos), integer(_integer) { }

  const string toString() const { return "(integer_keyword_statement)"; }
  int getSize() const { return 1; }
  const ObjType getType() const { return ASM_INT_KEYWORD_STATEMENT; }

  void emitCode(Bloat::iterator & position) {
    *(position++) = integer;
  }
};

struct asm_long_keyword_statement : asm_data_keyword_statement {
  long int longInteger;

  asm_long_keyword_statement(const YYLTYPE& pos, const long int _long)
    : asm_data_keyword_statement(pos), longInteger(_long) { }

  const string toString() const { return "(long_integer_keyword_statement)"; }
  int getSize() const { return 2; }
  const ObjType getType() const { return ASM_LONG_KEYWORD_STATEMENT; }

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
  int getSize() const { return 1; }
  const ObjType getType() const { return ASM_CHAR_KEYWORD_STATEMENT; }

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
  int getSize() const { return str.size(); }
  const ObjType getType() const { return ASM_STRING_KEYWORD_STATEMENT; }

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
  int getSize() const { return 1; }
  const ObjType getType() const { return ASM_REAL_KEYWORD_STATEMENT; }
  void emitCode(Bloat::iterator & position) {
    *(position++) = static_cast<int>(real);
  }
};


#endif
