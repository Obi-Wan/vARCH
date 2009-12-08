/* 
 * File:   AsmParser.cpp
 * Author: ben
 * 
 * Created on 27 agosto 2009, 14.26
 */

#include "AsmParser.h"

#include <sstream>
#include "../include/macros.h"

using namespace std;

//AsmParser::AsmParser(const AsmParser& orig) { }

AsmParser::~AsmParser() { }

inline void
AsmParser::preProcess() {
  istringstream fileStr(fileContent);

  // Scans over the lines
  for(unsigned int bytePos = 0; !fileStr.eof(); ) {
    vector<string> lineOfCode;
    string word, line;
    getline(fileStr, line);

    // let's skip blank/comment lines
    if (line[0] != ';' && !line.empty()) {
      istringstream lineStr(line);
      lineStr >> word;

      // First of all, let's test if it's a label or a type marker
      if (word[0] == '.') {
        if (word[word.size()-1] == ':') { // it's a label

          string label = word.substr(1,word.size()-2);

          if (labels.find(label) != labels.end()) {
            throw DuplicateLabelException(
                    "Label already defined here: " + labels.find(label)->second);
          }

          labels.insert(Labels::value_type(label, bytePos));
        } else { // it's a type marker
          /* Now we try to figure out what kind of marker it is:
           *  - if global is not declared, it should be something not related
           *    to heap;
           *  - otherwise print an error.
           */
          if (!globalConsts) {
            switch (getConstType(word)) {
              case GLOBAL:
                globalConsts = true;
                break;
              default:
                throw WrongIstructionException("No constants allowed here");
            }
          } else {
            /* Ok, we are in the heap and these are constants */
            switch (getConstType(word)) {
              case INT: {
                lineStr.ignore(256,'$');
                int value;
                lineStr >> value;
                consts.push_back(value);
                bytePos++;
                break;
              }
              case CHAR:
              case STRING: {
                lineStr.ignore(256,'$');
                string constString;
                getline(lineStr, constString);

                for (unsigned int i = 0; i < constString.size(); i++) {
                  consts.push_back(constString[i] & 0xff );
                  bytePos++;
                }
                break;
              }
              default:
                throw WrongArgumentException("Only constants after a .global mark");
            } // switch end
          }
        }
      } else { // it's a command
        if (!word.empty()) {
          lineOfCode.push_back(word);
          bytePos++;
          for(; (lineStr >> word) && !word.empty(); ) {
            lineOfCode.push_back(word);
            bytePos++;
          }
        }
      }
    }
    
    lines.push_back(lineOfCode);
  }

  DebugPrintfMaps(Labels, labels, "labels");
  //DebugPrintfMaps(Constants, consts, "consts");

  printf("PreProcess Finished\n");
}

void
AsmParser::parse() throw(WrongArgumentException, WrongIstructionException) {
  preProcess();

  int lineNum = 0;
  for(CodeLines::iterator line = lines.begin(); line != lines.end(); line++) {
    if (line->empty()) continue;
    
    try {
      int op = ISet.getIstr(line->at(0));

      vector<int> args;
      for(int i = 0; i < ((op >> 30) & 3); i++) {
        args.push_back(processArgOp(op, line->at(i+1), i));
      }
    
      code.push_back(op);
      for(int i = 0; i < args.size(); i++) {
        code.push_back(args[i]);
      }
    } catch (WrongIstructionException e) {

      stringstream streamError(string(""));
      streamError << "Line " << lineNum << ": ";
      e.prefixMessage(streamError.str());
      throw e;
    } catch (out_of_range ) {
      
      stringstream streamError(string(""));
      streamError << "Line " << lineNum << ": wrong number of arguments: "
                  << "expected more";
      throw WrongArgumentException(streamError.str());
    }
    
    lineNum++;
  }

  // pushing constants
  for (unsigned int i = 0; i < consts.size(); i++) {
    code.push_back(consts[i]);
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
      op += ARG(numArg, ADDR);
      if (labels.find(arg.substr(1, arg.size()-1)) == labels.end())
        throw WrongArgumentException("No label named " + arg);
      argValue = labels.find(arg.substr(1, arg.size()-1))->second;
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
      throw WrongArgumentException("Invalid Argument: " + arg);
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
      if (reg[1] == 'P') {
        return STACK_POINTER;
      } else if (reg[1] == 'R') {
        return STATE_REGISTER;
      } else {
        break;
      }
    case 'U':
      return USER_STACK_POINTER;
    default:
      break;
  }
  throw WrongArgumentException("No register called: " + reg);
}

inline AsmParser::ConstsType
AsmParser::getConstType(const string& type) const {
  switch (type[1]) {
    case 'i':
      return INT;
//    case 'l':
//      return LONG;
    case 'c':
      return CHAR;
    case 's':
      return STRING;
    case 'g':
      return GLOBAL;
    default:
      throw WrongArgumentException(
              "No constant type: " + type + " ");
  }
}