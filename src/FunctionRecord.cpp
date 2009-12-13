/* 
 * File:   FunctionRecord.cpp
 * Author: ben
 * 
 * Created on 10 dicembre 2009, 21.26
 */

#include "FunctionRecord.h"

//FunctionRecord::FunctionRecord() { }

//FunctionRecord::FunctionRecord(const FunctionRecord& orig) { }
//
//FunctionRecord::~FunctionRecord() { }

inline void
FunctionRecord::storeLabel(const string& word, const int& bytePos)
            throw(DuplicateLabelException)
{
  string label = word.substr(1,word.size()-2);

  if (localLabels.find(label) != localLabels.end()) {
    throw DuplicateLabelException(
            "Label already defined here: " + localLabels.find(label)->second);
  }
  localLabels.insert(Labels::value_type(label, bytePos));
}

inline void
FunctionRecord::lineParsingKernel( const CodeLines::const_iterator& line, Bloat& code)
              throw(WrongArgumentException, WrongIstructionException)
{
  try {
    int op = ISet.getIstr(line->second.at(0));

    vector<int> args;
    for(int i = 0; i < ((op >> 30) & 3); i++) {
      args.push_back(processArgOp(op, line->second.at(i+1), i));
    }

    code.push_back(op);
    for(int i = 0; i < args.size(); i++) {
      code.push_back(args[i]);
    }
  } catch (WrongIstructionException e) {

    stringstream streamError(string(""));
    streamError << "Line " << line->first << ": ";
    e.prefixMessage(streamError.str());
    throw e;
  } catch (out_of_range ) {

    stringstream streamError(string(""));
    streamError << "Line " << line->first << ": wrong number of arguments: "
                << "expected more";
    throw WrongArgumentException(streamError.str());
  }
}

inline int
FunctionRecord::processArgOp(int& op, const string& arg, const int& numArg) {

  int argValue = 0;
  switch (arg[0]) {
    case '$':
      op += ARG(numArg, COST);
      istringstream(arg.substr(1, arg.size()-1)) >> argValue;
      break;
    case '.':
      op += ARG(numArg, ADDR);
      if (localLabels.find(arg.substr(1, arg.size()-1)) == localLabels.end())
        throw WrongArgumentException("No label named " + arg);
      argValue = localLabels.find(arg.substr(1, arg.size()-1))->second;
      break;
    case '@':
      op += ARG(numArg, COST);
      if (localLabels.find(arg.substr(1, arg.size()-1)) == localLabels.end())
        throw WrongArgumentException("No label named " + arg);
      argValue = localLabels.find(arg.substr(1, arg.size()-1))->second;
      break;
    case '%':
      op += ARG(numArg, REG);
      argValue = parseReg(arg.substr(1, arg.size()-1));
      break;
    case '(':
      op += ARG(numArg, ADDR_IN_REG);
      argValue = parseReg(arg.substr(1, arg.size()-2));
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
FunctionRecord::parseReg(const string& reg) {
  int answer = 0;
  switch (reg[0]) {
    case 'R':
      istringstream(reg.substr(1, reg.size()-1)) >> answer;
      answer -= 1;
      break;
    case 'A':
      istringstream(reg.substr(1, reg.size()-1)) >> answer;
      answer += OFFSET_REGS - 1;
      break;
    case 'U':
      answer = USER_STACK_POINTER;
      break;
    case 'S':
      if (reg[1] == 'P') {
        answer = STACK_POINTER;
        break;
      } else if (reg[1] == 'R') {
        answer = STATE_REGISTER;
        break;
      }
    default:
      throw WrongArgumentException("No register called: " + reg);
  }
  return answer;
}
