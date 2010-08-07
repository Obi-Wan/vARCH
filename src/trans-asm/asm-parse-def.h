
#ifndef ASM_PARSE_DEF_H
#define ASM_PARSE_DEF_H

#include <vector>
#include <list>
#include <string>
#include <typeinfo>
#include <cstdio>
#include <cstring>
#include <map>

#include "asm_helpers.h"
#include "std_istructions.h"
#include "masks.h"
#include "exceptions.h"
#include "../FileHandler.h"

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
};

struct asm_immediate_arg : asm_arg {
  union {
    enum Registers regNum;
    int val;
    float fval;
  } content;

  const string toString() const {
    return "arg";
  }

  asm_immediate_arg() { }
  asm_immediate_arg(const int _val, const TypeOfArgument& type) : asm_arg(type) {
    content.val = _val;
  }
  asm_immediate_arg(const float _fval) : asm_arg(COST) {
    content.fval = _fval;
  }

  const int getCode() const { return content.val; }
};

struct asm_label_arg : asm_arg {
  string label;
  int pointedPosition;

  asm_label_arg(const string& _lab, const TypeOfArgument& _type)
    : asm_arg(_type), label(_lab) { }
  asm_label_arg(const char * _lab, const TypeOfArgument& _type)
    : asm_arg(_type), label(_lab) { }

  const string toString() const {
    return string("label: ").append(label);
  }

  const int getCode() const { return pointedPosition; }
};

struct asm_statement {
  int offset;

  YYLTYPE position;

  asm_statement() : offset(0) { }
  asm_statement(const int& _offs) : offset(_offs) { }

  virtual const string toString() const { return ""; }
  virtual bool isValid() const { return true; }
  virtual int getSize() const { return 0; }

  virtual void emitCode(vector<int>::iterator & position) { }
};

struct asm_instruction_statement : asm_statement {
  int instruction;
  vector<asm_arg *> args;

  asm_instruction_statement(const int _instr) : instruction(_instr) { }

  asm_instruction_statement * addArg(asm_arg * newArg) {
    args.push_back(newArg);
    newArg->relOffset = args.size();
    return this;
  }

  int getSize() const {
    return 1 + args.size();
  }

  const string toString() const {
    string output = "instr, ";
    for(int i = 0; i < args.size(); i++) {
      output += args[i]->toString() + " ";
    }
    return output;
  }

  vector<asm_label_arg *> getRefsToLabels() {
    vector<asm_label_arg *> refs;
    for(int i = 0; i < args.size(); i++) {
      if (typeid(*args[i]) == typeid(asm_label_arg)) {
        refs.push_back((asm_label_arg *)args[i]);
      }
    }
    return refs;
  }

  void emitCode(vector<int>::iterator & position) {
    int op = instruction;
    for (int i = 0; i < args.size(); i++) {
      op += ARG(i, (args[i]->type + (args[i]->relative ? RELATIVE_ARG : 0)) );
    }
    *(position) = op;
    position++;

    for (int i = 0; i < args.size(); i++) {
      *(position) = args[i]->getCode();
      position++;
    }
  }
};

struct asm_data_statement : asm_statement {
};

struct asm_null_statement : asm_data_statement {
  bool isValid() const { return false; }
};

struct asm_label_statement : asm_data_statement {
  string label;

  asm_label_statement(const char * _label) : label(_label) { }
  asm_label_statement(const string& _label) : label(_label) { }

  const string toString() const {
    return string("label: ").append(label);
  }
};

struct asm_data_keyword_statement : asm_data_statement {
  const string toString() const {
    return "data_keyword_statement";
  }
};

struct asm_int_keyword_statement : asm_data_keyword_statement {
  int integer;

  asm_int_keyword_statement(const int _integer) : integer(_integer) { }

  const string toString() const {
    return "integer_keyword_statement";
  }

  int getSize() const {
    return 1;
  }

  void emitCode(vector<int>::iterator & position) {
    *(position) = integer;
    position++;
  }
};

struct asm_long_keyword_statement : asm_data_keyword_statement {
  long int longInteger;

  asm_long_keyword_statement(const long int _long) : longInteger(_long) { }

  const string toString() const {
    return "long_integer_keyword_statement";
  }

  int getSize() const {
    return 2;
  }

  void emitCode(vector<int>::iterator & position) {
    *(position) = EXTRACT_HIGHER_SWORD_FROM_DWORD(longInteger);
    position++;
    *(position) = EXTRACT_LOWER__SWORD_FROM_DWORD(longInteger);
    position++;
  }
};

struct asm_char_keyword_statement : asm_data_keyword_statement {
  char character;

  asm_char_keyword_statement(const char * _char) : character(*(_char)) { }

  const string toString() const {
    return "char_keyword_statement";
  }

  int getSize() const {
    return 1;
  }

  void emitCode(vector<int>::iterator & position) {
    *(position) = (character & 0xff );
    position++;
  }
};

struct asm_string_keyword_statement : asm_data_keyword_statement {
  string str;

  asm_string_keyword_statement(const string& _str) : str(_str) { }

  const string toString() const {
    return  string("string_keyword_statement: ").append(str);
  }

  int getSize() const {
    return str.size();
  }

  void emitCode(vector<int>::iterator & position) {
    for (int i = 0; i < str.size(); i++) {
      *(position) = (str[i] & 0xff );
      position++;
    }
  }
};

struct asm_real_keyword_statement : asm_data_keyword_statement {
  float real;

  asm_real_keyword_statement(const float _real) : real(_real) { }

  int getSize() const {
    return 1;
  }

  void emitCode(vector<int>::iterator & position) {
    *(position) = static_cast<int>(real);
    position++;
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
    printf("TableOfSymbols: adding label at position %03d: %s\n", lab->offset, lab->label.c_str());
    defLabels.insert( LabelsMap::value_type( lab->label, labelRecord(lab) ) );
  }

  int getPositionOfLabel(const string& name) {
    LabelsMap::iterator iter = defLabels.find(name);
    if (iter != defLabels.end()) {
      return iter->second.label->offset;
    } else return -1;
  }
};

static TableOfSymbols globalSymbols;

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
  int functionOffset;

  bool isLocalRegion;
  int tempLocalOffset;

  vector<asm_statement *> stmts;
  vector<asm_data_statement *> locals;

  TableOfSymbols localSymbols;
  list<argLabelRecord *> refs;

  asm_function(const string& _name, const int offs)
              : name(_name), functionOffset(offs), isLocalRegion(false),
                tempLocalOffset(0) { }
  asm_function(const char * _name, const int offs)
              : name(_name), functionOffset(offs), isLocalRegion(false),
                tempLocalOffset(0) { }

  void addStmt(asm_statement * stmt) {
    if (!isLocalRegion) {
      stmt->offset = tempLocalOffset;
      tempLocalOffset += stmt->getSize();
      stmts.push_back(stmt);

      if (typeid(*stmt) == typeid(asm_instruction_statement)) {
        vector<asm_label_arg *> refsOfStmt = ((asm_instruction_statement *)stmt)->getRefsToLabels();
        for (int i = 0; i < refsOfStmt.size(); i++) {
          argLabelRecord * tempRecord = new argLabelRecord();
          tempRecord->arg = refsOfStmt[i];
          tempRecord->parent = stmt;
          refs.push_back(tempRecord);
        }
      }
      checkLabel(stmt);

    } else {
      throw WrongIstructionException(
                "No instructions should be in local region");
    }
  }
  
  void checkLabel(asm_statement * stmt) {
    if (typeid(*stmt) == typeid(asm_label_statement)) {
      printf("Found local label: %s in function %s!\n",
             ((asm_label_statement *)stmt)->label.c_str(), name.c_str());
      localSymbols.addLabel((asm_label_statement *)stmt);
    }
  }

  void addDataStmt(asm_data_statement * stmt) {
    stmt->offset = tempLocalOffset;
    tempLocalOffset += stmt->getSize();
    locals.push_back(stmt);
    checkLabel(stmt);
  }

  void enterLocalRegion() { isLocalRegion = true; }

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

static struct asm_program {
  bool isGlobalRegion;
  int tempOffset;

  vector<asm_function *> functions;
  vector<asm_data_statement *> globals;

  asm_function * currentFunction;

  asm_program() : isGlobalRegion(false), currentFunction(NULL), tempOffset(0) { }

  void addStmt(asm_statement * stmt)
        throw (WrongIstructionException)
  {
    if (stmt->isValid()) {
      if (!isGlobalRegion) {
        if (currentFunction == NULL) {
          if (!functions.empty()) {
            throw WrongIstructionException();
          }
          printf("Creating main function\n");
          
          entryNewFunction("main");
        }
        currentFunction->addStmt(stmt);
      } else {
        throw WrongIstructionException(
                "No instructions should be in global region");
      }
    }
  }

  void addDataStmt(asm_data_statement * stmt) {
    if (stmt->isValid()) {
      if (!isGlobalRegion) {
        if (currentFunction == NULL || currentFunction->stmts.size() == 0) {
          throw WrongIstructionException("Data statements should not be in instructions region");
        } else {
          currentFunction->addDataStmt(stmt);
        }
      } else {
        stmt->offset = tempOffset;
        tempOffset += stmt->getSize();
        globals.push_back(stmt);

        if (typeid(*stmt) == typeid(asm_label_statement)) {
          printf("Found label statement: %s!\n", ((asm_label_statement *)stmt)->label.c_str());
          globalSymbols.addLabel((asm_label_statement *)stmt);
        }
      }
    }
  }

  void enterLocalRegionCurrentFunction() {
    if (currentFunction != NULL) {
      currentFunction->enterLocalRegion();
    } else {
      throw WrongIstructionException("Not in a Function!");
    }
  }

  void closeCurrentFunction() {
    functions.push_back(currentFunction);
    tempOffset += currentFunction->getSize();
    currentFunction = NULL;
  }

  void entryNewFunction(const string& name) throw (WrongIstructionException) {
    if (currentFunction == NULL) {
      currentFunction = new asm_function(name, tempOffset);
      asm_label_statement * tempLabel = new asm_label_statement(name);
      tempLabel->offset = tempOffset;
      currentFunction->stmts.push_back(tempLabel);
      globalSymbols.addLabel(tempLabel);
    } else {
      throw WrongIstructionException("Already opened a function");
    }
  }

  int getFunciontsTotalSize() {
    int totSize = 0;
    for(int i = 0; i < functions.size(); i++) {
      totSize += functions[i]->getSize();
    }
    return totSize;
  }

  void enterGlobalRegion() {
    if (currentFunction != NULL) {
      closeCurrentFunction();
    }
    isGlobalRegion = true;
  }
  
  void assignValuesToLabels() {
    printf("Assigning labels:\n");
    for(int i = 0; i < functions.size(); i++) {

      printf("Processing refs of function: %s\n",functions[i]->name.c_str());
      for(list<argLabelRecord *>::iterator j  = functions[i]->refs.begin();
              j != functions[i]->refs.end(); j++) {
        printf("Processing label: %s\n", (*(j))->arg->label.c_str());
        int pos = functions[i]->localSymbols.getPositionOfLabel((*(j))->arg->label);
        if (pos < 0) {
          printf("It's not local since it returned: %3d\n", pos);
          pos = globalSymbols.getPositionOfLabel((*(j))->arg->label);
          if (pos < 0) {
            printf("It's not even global! ERROR!!\n");
            throw WrongArgumentException("reference to a Label called: " +
                    (*(j))->arg->label + "that does not exist");
          } else {
            printf("It's global and at: %3d\n", pos);
            (*(j))->arg->pointedPosition = pos;
            (*(j))->arg->relative = false;
          }
        } else {
          printf("It's local since it returned: %3d\n", pos);
          printf("Calculating sizes:\n position of stmt: %03d\n", (*(j))->parent->offset);
          printf(" size of statement and args: %03d\n", (*(j))->parent->getSize());
          printf(" position of the arg relative to the stmt: %03d\n", (*(j))->arg->relOffset);
          (*(j))->arg->pointedPosition = pos - (*(j))->arg->relOffset - (*(j))->parent->offset;
          (*(j))->arg->relative = true;
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

    printf("Size of the generated file: %3d - follows the code\n", (int)bytecode.size());
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
} program;


#endif
