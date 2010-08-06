
#ifndef ASM_PARSE_DEF_H
#define ASM_PARSE_DEF_H

#include <vector>
#include <list>
#include <string>
#include <cstdio>
#include <cstring>
#include <map>

#include "asm_helpers.h"
#include "std_istructions.h"
#include "masks.h"
#include "exceptions.h"
#include "../src/FileHandler.h"

using namespace std;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

enum ObjType {
  ASM_ARG,
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

  asm_arg(const TypeOfArgument& _type) : type(_type), relOffset(0),
                                          relative(false) { }
  asm_arg() : relOffset(0), relative(false) { }
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

  asm_immediate_arg() { }
  asm_immediate_arg(const int _val, const TypeOfArgument& type)
      : asm_arg(type) {
    content.val = _val;
  }
  asm_immediate_arg(const float _fval) : asm_arg(COST) {
    content.fval = _fval;
  }

  const int getCode() const { return content.val; }
  const ObjType getType() const { return ASM_IMMEDIATE_ARG; }
  const string toString() const { return "arg"; }
};

struct asm_label_arg : asm_arg {
  string label;
  int pointedPosition;

  asm_label_arg(const string& _lab, const TypeOfArgument& _type)
    : asm_arg(_type), label(_lab) { }
  asm_label_arg(const char * _lab, const TypeOfArgument& _type)
    : asm_arg(_type), label(_lab) { }

  const string toString() const { return string("label: ").append(label); }
  const int getCode() const { return pointedPosition; }
  const ObjType getType() const { return ASM_LABEL_ARG; }
};

struct asm_statement {
  int offset;

  YYLTYPE position;

  asm_statement() : offset(0) { }
  asm_statement(const int& _offs) : offset(_offs) { }

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

  asm_instruction_statement(const int _instr) : instruction(_instr) { }
  asm_instruction_statement * addArg(asm_arg * newArg) {
    args.push_back(newArg);
    newArg->relOffset = args.size();
    return this;
  }

  const string toString() const {
    string output = "instr, ";
    for(int i = 0; i < args.size(); i++) {
      output += args[i]->toString() + " ";
    }
    return output;
  }

  void emitCode(Bloat::iterator & position) {
    int op = instruction;
    for (int i = 0; i < args.size(); i++) {
      op += ARG(i, (args[i]->type + (args[i]->relative ? RELATIVE_ARG : 0)) );
    }
    *(position++) = op;

    for (int i = 0; i < args.size(); i++) {
      *(position++) = args[i]->getCode();
    }
  }

  int getSize() const { return 1 + args.size(); }
  const ObjType getType() const { return ASM_INSTRUCTION_STATEMENT; }
};

///////////////////////
///////////////////////

struct asm_data_statement : asm_statement {
  const ObjType getType() const { return ASM_DATA_STATEMENT; }
};

struct asm_label_statement : asm_data_statement {
  string label;

  asm_label_statement(const char * _label) : label(_label) { }
  asm_label_statement(const string& _label) : label(_label) { }

  const string toString() const { return string("label: ").append(label); }
  const ObjType getType() const { return ASM_LABEL_STATEMENT; }
};

struct asm_data_keyword_statement : asm_data_statement {
  const string toString() const { return "data_keyword_statement"; }
  const ObjType getType() const { return ASM_KEYWORD_STATEMENT; }
};

struct asm_int_keyword_statement : asm_data_keyword_statement {
  int integer;

  asm_int_keyword_statement(const int _integer) : integer(_integer) { }

  const string toString() const { return "integer_keyword_statement"; }
  int getSize() const { return 1; }
  const ObjType getType() const { return ASM_INT_KEYWORD_STATEMENT; }

  void emitCode(Bloat::iterator & position) {
    *(position++) = integer;
  }
};

struct asm_long_keyword_statement : asm_data_keyword_statement {
  long int longInteger;

  asm_long_keyword_statement(const long int _long) : longInteger(_long) { }

  const string toString() const { return "long_integer_keyword_statement"; }
  int getSize() const { return 2; }
  const ObjType getType() const { return ASM_LONG_KEYWORD_STATEMENT; }

  void emitCode(Bloat::iterator & position) {
    *(position++) = EXTRACT_HIGHER_SWORD_FROM_DWORD(longInteger);
    *(position++) = EXTRACT_LOWER__SWORD_FROM_DWORD(longInteger);
  }
};

struct asm_char_keyword_statement : asm_data_keyword_statement {
  char character;

  asm_char_keyword_statement(const char * _char) : character(*(_char)) { }

  const string toString() const { return "char_keyword_statement"; }
  int getSize() const { return 1; }
  const ObjType getType() const { return ASM_CHAR_KEYWORD_STATEMENT; }

  void emitCode(Bloat::iterator & position) {
    *(position++) = (character & 0xff );
  }
};

struct asm_string_keyword_statement : asm_data_keyword_statement {
  string str;

  asm_string_keyword_statement(const string& _str) : str(_str) { }

  const string toString() const {
    return  string("string_keyword_statement: ").append(str);
  }
  int getSize() const { return str.size(); }
  const ObjType getType() const { return ASM_STRING_KEYWORD_STATEMENT; }

  void emitCode(Bloat::iterator & position) {
    for (int i = 0; i < str.size(); i++) {
      *(position++) = (str[i] & 0xff );
    }
  }
};

struct asm_real_keyword_statement : asm_data_keyword_statement {
  float real;

  asm_real_keyword_statement(const float _real) : real(_real) { }

  const string toString() const { return  "real_keyword_statement: "; }
  int getSize() const { return 1; }
  const ObjType getType() const { return ASM_REAL_KEYWORD_STATEMENT; }
  void emitCode(Bloat::iterator & position) {
    *(position++) = static_cast<int>(real);
  }
};


///////////////////////
// Labels Management //
///////////////////////

struct labelRecord {
  asm_statement * pointedStatement;
  asm_label_statement * label;

  labelRecord(asm_label_statement* lab) : label(lab), pointedStatement(NULL) { }
};

typedef map<string, labelRecord> LabelsMap;

class TableOfSymbols {
   LabelsMap defLabels;
public:
  void addLabel(asm_label_statement* lab) {
    printf("TableOfSymbols: adding label at position %03d: %s\n",
           lab->offset, lab->label.c_str());
    defLabels.insert( LabelsMap::value_type( lab->label, labelRecord(lab) ) );
  }

  int getPositionOfLabel(const string& name) {
    LabelsMap::iterator iter = defLabels.find(name);
    if (iter != defLabels.end()) {
      return iter->second.label->offset;
    } else return -1;
  }
};

struct argLabelRecord {
  asm_statement * parent;
  asm_label_arg * arg;
};

//typedef pair<asm_statement *, asm_label_arg *> argLabelRecord;

///////////////////////////
// Functions             //
///////////////////////////

struct asm_function {
  const string name;
  int tempLocalOffset;
  int funcOffset;

  vector<asm_statement *> stmts;
  vector<asm_data_statement *> locals;

  TableOfSymbols localSymbols;
  list<argLabelRecord *> refs;

  asm_function(const string& _name, list<asm_statement *> * _stmts,
               list<asm_data_statement *> * _locals)
              : name(_name), tempLocalOffset(0), funcOffset(0)
  { copyStmts(_stmts, _locals); fixStmts(); }
  asm_function(const char * _name, list<asm_statement *> * _stmts,
               list<asm_data_statement *> * _locals)
              : name(_name), tempLocalOffset(0), funcOffset(0)
  { copyStmts(_stmts, _locals); fixStmts(); }

  void copyStmts(list<asm_statement *> * _stmts,
                 list<asm_data_statement *> * _locals) {
    stmts.reserve(_stmts->size());
    stmts.insert(stmts.begin(), _stmts->begin(), _stmts->end());
    locals.reserve(_locals->size());
    locals.insert(locals.begin(), _locals->begin(), _locals->end());
  }

  void fixStmts() {
    for(int i = 0; i < stmts.size(); i++) {
      asm_statement * stmt = stmts[i];

      stmt->offset = tempLocalOffset;
      tempLocalOffset += stmt->getSize();

      if (stmt->getType() == ASM_INSTRUCTION_STATEMENT) {
        asm_instruction_statement * istmt = (asm_instruction_statement *) stmt;
        for (int i = 0; i < istmt->args.size(); i++) {
          if (istmt->args[i]->getType() == ASM_LABEL_ARG) {
            argLabelRecord * tempRecord = new argLabelRecord();
            tempRecord->arg = (asm_label_arg *)istmt->args[i];
            tempRecord->parent = istmt;
            refs.push_back(tempRecord);
          }
        }
      } else {
        checkLabel(stmt);
      }
    }
    for(int i = 0; i < locals.size(); i++) {
      asm_data_statement * stmt = locals[i];

      stmt->offset = tempLocalOffset;
      tempLocalOffset += stmt->getSize();
      checkLabel(stmt);
    }
  }

  void checkLabel(asm_statement * stmt) {
    if (stmt->getType() == ASM_LABEL_STATEMENT) {
      printf("Found local label: %s in function %s!\n",
             ((asm_label_statement *)stmt)->label.c_str(), name.c_str());
      localSymbols.addLabel((asm_label_statement *)stmt);
    }
  }

  int getInstrSize() const {
    int size = 0;
    for(int i = 0; i < stmts.size(); i++) {
      size += stmts[i]->getSize();
    }
    return size;
  }
  int getDataSize() const {
    int size = 0;
    for(int i = 0; i < locals.size(); i++) {
      size += locals[i]->getSize();
    }
    return size;
  }
  int getSize() const { return getInstrSize() + getDataSize(); }
};

///////////////////////////
// Program               //
///////////////////////////

struct asm_program {
  int tempOffset;

  vector<asm_function *> functions;
  vector<asm_data_statement *> globals;

  TableOfSymbols globalSymbols;

  asm_program(list<asm_function *> * _funcs,
              list<asm_data_statement *> * _globals) : tempOffset(0)
  {
    functions.reserve(_funcs->size());
    functions.insert(functions.begin(), _funcs->begin(), _funcs->end());
    globals.reserve(_globals->size());
    globals.insert(globals.begin(), _globals->begin(), _globals->end());

    for(int i = 0; i < functions.size(); i++) {
      asm_function * func = functions[i];

      asm_label_statement * tempLabel = new asm_label_statement(func->name);
      tempLabel->offset = tempOffset;

      globalSymbols.addLabel(tempLabel);

      tempOffset += func->getSize();
    }
    for(int i = 0; i < globals.size(); i++) {
      asm_data_statement * stmt = globals[i];

      stmt->offset = tempOffset;
      tempOffset += stmt->getSize();
      if (stmt->getType() == ASM_LABEL_STATEMENT) {
        printf("Found label statement: %s!\n",
               ((asm_label_statement *)stmt)->label.c_str());
        globalSymbols.addLabel((asm_label_statement *)stmt);
      }
    }
  }

  int getFunciontsTotalSize() {
    int totSize = 0;
    for(int i = 0; i < functions.size(); i++) {
      totSize += functions[i]->getSize();
    }
    return totSize;
  }

  void assignValuesToLabels() {
    printf("Assigning labels:\n");
    for(int i = 0; i < functions.size(); i++) {

      printf("Processing refs of function: %s\n",functions[i]->name.c_str());
      for(list<argLabelRecord *>::iterator j  = functions[i]->refs.begin();
              j != functions[i]->refs.end(); j++) {
        printf("Processing label: %s\n", (*j)->arg->label.c_str());
        int pos = functions[i]->localSymbols.getPositionOfLabel((*j)->arg->label);
        if (pos < 0) {
          printf("It's not local since it returned: %3d\n", pos);
          pos = globalSymbols.getPositionOfLabel((*j)->arg->label);
          if (pos < 0) {
            printf("It's not even global (returned %3d)! ERROR!!\n", pos);
            throw WrongArgumentException("reference to a Label called: " +
                    (*j)->arg->label + " that does not exist");
          } else {
            printf("It's global and at: %3d\n", pos);
            (*j)->arg->pointedPosition = pos;
            (*j)->arg->relative = false;
          }
        } else {
          printf("It's local since it returned: %3d\n", pos);
          printf("Calculating sizes:\n position of stmt: %03d\n", (*j)->parent->offset);
          printf(" size of statement and args: %03d\n", (*j)->parent->getSize());
          printf(" position of the arg relative to the stmt: %03d\n", (*j)->arg->relOffset);
          (*j)->arg->pointedPosition = pos - (*j)->arg->relOffset - (*j)->parent->offset;
          (*j)->arg->relative = true;
        }
      }
    }
  }

  void assemble(const char * inputFile) {
    Bloat bytecode;
    bytecode.resize(tempOffset,0);
    Bloat::iterator pos = bytecode.begin();
    for (int i = 0; i < functions.size(); i++) {
      for(int j = 0; j < functions[i]->stmts.size(); j++) {
        functions[i]->stmts[j]->emitCode(pos);
      }
      for(int j = 0; j < functions[i]->locals.size(); j++) {
        functions[i]->locals[j]->emitCode(pos);
      }
    }
    for (int i = 0; i < globals.size(); i++) {
      globals[i]->emitCode(pos);
    }

    printf("Size of the generated file: %3d - follows the code\n",
           (int)bytecode.size());
    for (int i = 0; i < bytecode.size(); i++) {
      printf("Mem %03d: %12d\n", i, bytecode[i]);
    }

    const int inputNameLen = strlen(inputFile);
    char outfile[inputNameLen + 3];
    strcpy(outfile, inputFile);
    outfile[inputNameLen-1] = 'b';
    outfile[inputNameLen] = 'i';
    outfile[inputNameLen+1] = 'n';
    outfile[inputNameLen+2] = '\0';

    BinWriter writer(outfile);
    writer.saveBinFileContent(bytecode);
  }
};

#endif
