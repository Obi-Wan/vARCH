/* 
 * File:   AsmParser.cpp
 * Author: ben
 * 
 * Created on 27 agosto 2009, 14.26
 */

#include "AsmParser.h"

#include <sstream>

#define PUT_ISTR( x ) istructions.insert(Istructions::value_type(NAME_OF( x ), x ))

using namespace std;

//AsmParser::AsmParser(const AsmParser& orig) { }

AsmParser::~AsmParser() { }

void
AsmParser::init() {
  PUT_ISTR(SLEEP);
  PUT_ISTR(PUSHA);
  PUT_ISTR(POPA);
  PUT_ISTR(RET);
  PUT_ISTR(REBOOT);
  PUT_ISTR(HALT);

  PUT_ISTR(NOT);
  PUT_ISTR(INCR);
  PUT_ISTR(DECR);
  PUT_ISTR(COMP2);
  PUT_ISTR(LSH);
  PUT_ISTR(RSH);
  PUT_ISTR(STACK);
  PUT_ISTR(PUSH);
  PUT_ISTR(POP);
  PUT_ISTR(JSR);
  PUT_ISTR(JMP);
  PUT_ISTR(IFJ);
  PUT_ISTR(IFNJ);

  PUT_ISTR(MOV);
  PUT_ISTR(ADD);
  PUT_ISTR(MULT);
  PUT_ISTR(SUB);
  PUT_ISTR(DIV);
  PUT_ISTR(QUOT);
  PUT_ISTR(AND);
  PUT_ISTR(OR);
  PUT_ISTR(XOR);
  PUT_ISTR(MMU);
  PUT_ISTR(PUT);
  PUT_ISTR(GET);
  PUT_ISTR(EQ);
  PUT_ISTR(LO);
  PUT_ISTR(MO);
  PUT_ISTR(LE);
  PUT_ISTR(LE);
  PUT_ISTR(ME);
  PUT_ISTR(NEQ);
  
  PUT_ISTR(BPUT);
  PUT_ISTR(BGET);
  PUT_ISTR(IFEQJ);
  PUT_ISTR(IFNEQJ);
  PUT_ISTR(IFLOJ);
  PUT_ISTR(IFMOJ);
  PUT_ISTR(IFLEJ);
  PUT_ISTR(IFMEJ);

  #ifdef DEBUG
  for(Istructions::iterator iter = istructions.begin(); iter != istructions.end(); iter++) {
    printf("DEBUG: Istr name: %s, value: %d\n", iter->first.c_str(), iter->second);
  }
  #endif
}

inline void
AsmParser::preProcess() {
  istringstream fileStr(fileContent);
  string newFileContent;

  for(unsigned int i = 0; !fileStr.eof(); ) {  
    string word, line;
    getline(fileStr, line);

    if (line[0] != ';' && !line.empty()) {
      istringstream lineStr(line);
      lineStr >> word;

      if (word[word.size()-1] == ':') {

        string label = word.substr(0,word.size()-1);

        if (labels.find(label) != labels.end()) {
          throw DuplicateLabelException(
                  "Label already defined here: " + labels.find(label)->second);
        }

        labels.insert(Labels::value_type(label, i));
      } else if (word == "DC") {
        string name;
        lineStr >> name;

        if (consts.find(name) != consts.end()) {
          throw DuplicateConstException(
                  "Const already defined with the name: " + consts.find(name)->first);
        }

        lineStr.ignore(256,'$');
        int value;
        lineStr >> value;

        consts.insert(Constants::value_type(name,value));
      } else {
        i++;
        for(; (lineStr >> word) && !word.empty() ;) i++;

        newFileContent += line;
        newFileContent += '\n';
      }
    }
  }

  fileContent = newFileContent;

  #ifdef DEBUG
  printf("DEBUG: labels:\n");
  for(Labels::iterator i = labels.begin(); i != labels.end(); i++) {
    printf("label name: \"%s\", pos: %d\n", i->first.data(), i->second);
  }

  printf("DEBUG: consts:\n");
  for(Constants::iterator i = consts.begin(); i != consts.end(); i++) {
    printf("const name: \"%s\", value: %d\n", i->first.data(), i->second);
  }
  #endif
}

void
AsmParser::parse() throw(WrongArgumentException, WrongIstructionException) {
  preProcess();

  istringstream fileStr(fileContent);
  string line;

  for(int lineNum = 0;
      !fileStr.eof() && getline(fileStr, line) && !line.empty();
      lineNum++ )
  {
    istringstream lineStr(line);
    string word;
    lineStr >> word;
    Istructions::iterator iter = istructions.find(word);

    if (iter == istructions.end()) {
      stringstream streamError(string(""));
      streamError << "Line " << lineNum;
      throw WrongIstructionException(
              streamError.str() + ": Not existing Istruction: " + word);
    }

    int op = iter->second;
    vector<int> args;

    try {
      for(int i = 0; i < ((op >> 30) & 3); i++) {
        string arg;
        lineStr >> arg;
        args.push_back(processArgOp(op, arg, i));
      }
      
    } catch (WrongArgumentException e) {
      
      stringstream streamError(string(""));
      streamError << "Line " << lineNum;
      e.prefixMessage(streamError.str() + ": Wrong Argument! ");
      throw e;
    }
    
    code.push_back(op);
    for(int i = 0; i < args.size(); i++) {
      code.push_back(args[i]);
    }
  }
}

inline int
AsmParser::processArgOp(int& op, const string& arg, const int& numArg) {

  int argValue = 0;
  switch (arg[0]) {
    case '$':
      op += ARG(numArg, COST);
      istringstream(arg.substr(1, arg.size()-1)) >> argValue;
      break;
    case '.':
      op += ARG(numArg, COST);
      if (consts.find(arg.substr(1, arg.size()-1)) == consts.end())
        throw WrongArgumentException("No costant named " + arg);
      argValue = consts.find(arg.substr(1, arg.size()-1))->second;
      break;
    case '%':
      op += ARG(numArg, REG);
      argValue = parseReg(arg.substr(1, arg.size()-1)) - 1;
      break;
    case '(':
      op += ARG(numArg, ADDR_IN_REG);
      argValue = parseReg(arg.substr(1, arg.size()-2)) - 1;
      break;
    case '>':
      op += ARG(numArg, ADDR);
      istringstream(arg.substr(1, arg.size()-1)) >> argValue;
      break;
    default:
      throw WrongArgumentException();
  }
  return argValue;
}

inline int
AsmParser::parseReg(const string& reg) {
  int temp = 0;
  switch (reg[0]) {
    case 'R':
      istringstream(reg.substr(1, reg.size()-1)) >> temp;
      return temp;
    case 'A':
      istringstream(reg.substr(1, reg.size()-1)) >> temp;
      return (temp += OFFSET_REGS);
    case 'S':
      return STACK_POINTER;
    case 'U':
      return USER_STACK_POINTER;
    default:
      throw WrongArgumentException();
  }
}
