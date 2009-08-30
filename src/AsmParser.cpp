/* 
 * File:   AsmParser.cpp
 * Author: ben
 * 
 * Created on 27 agosto 2009, 14.26
 */

#include "AsmParser.h"

#include <sstream>
#include "../include/macros.h"

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
  PUT_ISTR(TCJ);
  PUT_ISTR(TZJ);
  PUT_ISTR(TOJ);
  PUT_ISTR(TNJ);
  PUT_ISTR(TSJ);

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

  DebugPrintfMaps(Istructions, istructions, "istructions");
}

inline void
AsmParser::preProcess() {
  istringstream fileStr(fileContent);

  for(unsigned int i = 0; !fileStr.eof(); ) {
    vector<string> lineOfCode;
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
        if (!word.empty()) {
          lineOfCode.push_back(word);
          i++;
          for(; (lineStr >> word) && !word.empty(); ) {
            lineOfCode.push_back(word);
            i++;
          }
        }
      }
    }
    
    lines.push_back(lineOfCode);
  }

  DebugPrintfMaps(Labels, labels, "labels");
  DebugPrintfMaps(Constants, consts, "consts");

  printf("PreProcess Finished\n");
}

void
AsmParser::parse() throw(WrongArgumentException, WrongIstructionException) {
  preProcess();

  int lineNum = 0;
  for(CodeLines::iterator line = lines.begin(); line != lines.end(); line++) {
    if (line->empty()) continue;
    
    Istructions::iterator istr = istructions.find( line->at(0) );
    
    if (istr == istructions.end()) {
      stringstream streamError(string(""));
      streamError << "Line " << lineNum << ": "
                  << "Not existing Istruction: " << line->at(0);
      throw WrongIstructionException( streamError.str() );
    }

    int op = istr->second;
    vector<int> args;

    try {
      for(int i = 0; i < ((op >> 30) & 3); i++) {
        args.push_back(processArgOp(op, line->at(i+1), i));
      }
    } catch (WrongArgumentException e) {
      
      stringstream streamError(string(""));
      streamError << "Line " << lineNum << ": Wrong Argument! ";
      e.prefixMessage(streamError.str());
      throw e;
    } catch (out_of_range ) {
      
      stringstream streamError(string(""));
      streamError << "Line " << lineNum << ": wrong number of arguments: "
                  << "expected more";
      throw WrongArgumentException(streamError.str());
    }
    
    code.push_back(op);
    for(int i = 0; i < args.size(); i++) {
      code.push_back(args[i]);
    }
    
    lineNum++;
  }

  printf("Parsing Finished\n");
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
    case '@':
      op += ARG(numArg, COST);
      if (labels.find(arg.substr(1, arg.size()-1)) == labels.end())
        throw WrongArgumentException("No label named " + arg);
      argValue = labels.find(arg.substr(1, arg.size()-1))->second;
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
