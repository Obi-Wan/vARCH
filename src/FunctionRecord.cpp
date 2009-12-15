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
FunctionRecord::storeLabel(const string& word, const int& bytePos, const int& lineNum)
            throw(DuplicateLabelException)
{
  string label = word.substr(1,word.size()-2);

  if (localLabels.find(label) != localLabels.end()) {
    stringstream streamError(string(""));
    streamError << "Line " << lineNum << ": Label already defined at line: "
                << localLabels.find(label)->second.lineNumber;
    throw DuplicateLabelException(streamError.str());
  }
  localLabels.insert(Labels::value_type(label, Label(word, lineNum, bytePos, 0)));
}

void
FunctionRecord::parseLocalSymbols() throw(WrongIstructionException, WrongArgumentException) {
  unsigned int bytePos = 0;
  bool localConsts = false;

  DebugPrintf(("Parsing Symbols of: %s\n", name.c_str()));
  for(CodeLines::const_iterator line = linesIntermed.begin();
          line != linesIntermed.end(); line++ ) {
    const string& word = line->chunks.at(0);
    if (word[0] == '.') {
      if (word[word.size()-1] == ':') { // it's a label
        storeLabel(word, bytePos, line->lineNumber);
      } else { // it's a type marker
        if (!localConsts) {
          if (Marker::getMarkerType(word) == Marker::LOCAL) {
            localConsts = true;
          } else {
            DebugPrintf(("marker: %s\n", word.c_str()));
            throw WrongIstructionException("No constants allowed here");
          }
        } else { /* Ok, we are processing constants */
          switch (Marker::getMarkerType(word)) {
            case Marker::INT: {
              istringstream argStr( line->chunks.at(1) );
              int value;
              argStr >> value;
              consts.push_back(value);
              bytePos++;
              break;
            }
            case Marker::CHAR:
            case Marker::STRING: {
              const string& temp = line->chunks.at(1);
              for (unsigned int i = 0; i < temp.size(); i++) {
                consts.push_back(temp[i] & 0xff );
                bytePos++;
              }
              break;
            }
            default:
              throw WrongArgumentException("Only constants after a .global mark");
          } // switch end
        }
      }
    } else {
      lines.push_back(*line);
      bytePos += line->bytes;
    }
  }

  DebugPrintfLabels( localLabels, "localLabels");
}

void
FunctionRecord::assemble() {
  unsigned int bytePos = 0;
  DebugPrintf(("Function %s assembled\n", name.c_str()));
  for(CodeLines::const_iterator line = lines.begin(); line != lines.end(); line++) {
    lineAssembleKernel(line, code, bytePos);
    bytePos += line->bytes;
  }
  DebugPrintf(("Istructions bytes: %d\n", bytePos));
  for (unsigned int i = 0; i < consts.size(); i++) {
    code.push_back(consts[i]);
  }
  DebugPrintf(("Total bytes: %d\n", (int)code.size()));
}

inline void
FunctionRecord::lineAssembleKernel( const CodeLines::const_iterator& line,
                                  Bloat& code, int bytePos)
              throw(WrongArgumentException, WrongIstructionException)
{
  try {
    int op = ISet.getIstr(line->chunks.at(0));

    vector<int> args;
    for(int i = 0; i < ((op >> 30) & 3); i++) {
      // Pre incrementing bytePos, since args will be after op, but op was not counted now
      args.push_back(processArgOp(op, line->chunks.at(i+1), i, ++bytePos));
    }

    code.push_back(op);
    for(int i = 0; i < args.size(); i++) {
      code.push_back(args[i]);
    }
  } catch (WrongIstructionException e) {

    stringstream streamError(string(""));
    streamError << "Line " << line->lineNumber << ": ";
    e.prefixMessage(streamError.str());
    throw e;
  } catch (out_of_range ) {

    stringstream streamError(string(""));
    streamError << "Line " << line->lineNumber << ": wrong number of arguments: "
                << "expected more";
    throw WrongArgumentException(streamError.str());
  }
}

// This helps just to keep clean/readable code
#define MANAGE_OPTIONS( x ) (options ? (( x << 2 ) + 0x1f & options) : x )

inline int
FunctionRecord::processArgOp(int& op, const string& arg, const int& numArg,
                             const int& bytePos)
                             throw(WrongArgumentException)
{
  int argValue = 0;

  string cleanArg = arg;
  int options = 0;
  detectArgsOptions(arg, cleanArg, options);

  switch (cleanArg[0]) {
    case '$': {
      op += ARG(numArg, MANAGE_OPTIONS(COST) );
      istringstream(cleanArg.substr(1, cleanArg.size()-1)) >> argValue;
      break;
    }
    case '.': {
      Labels::const_iterator iter = localLabels.find(cleanArg.substr(1, cleanArg.size()-1));
      if (iter != localLabels.end()) {
        op += ARG(numArg, ADDR + RELATIVE_ARG);
        argValue = iter->second.byte - bytePos;
      } else {
        iter = globalLabels.find(cleanArg.substr(1, cleanArg.size()-1));
        if (iter != globalLabels.end()) {
          op += ARG(numArg, ADDR);
          argValue = iter->second.byte;
        } else {
          throw WrongArgumentException("No label named " + cleanArg);
        }
      }
      break;
    }
    case '@': {
      Labels::const_iterator iter = localLabels.find(cleanArg.substr(1, cleanArg.size()-1));
      if (iter != localLabels.end()) {
        op += ARG(numArg, COST + RELATIVE_ARG);
        argValue = iter->second.byte - bytePos;
      } else {
        iter = globalLabels.find(cleanArg.substr(1, cleanArg.size()-1));
        if (iter != globalLabels.end()) {
          op += ARG(numArg, COST);
          argValue = iter->second.byte;
        } else {
          throw WrongArgumentException("No label named " + cleanArg);
        }
      }
      break;
    }
    case '%': {
      op += ARG(numArg, MANAGE_OPTIONS(REG) );
      argValue = parseReg(cleanArg.substr(1, cleanArg.size()-1));
      break;
    }
    case '(': {
      op += ARG(numArg, MANAGE_OPTIONS(ADDR_IN_REG) );
      argValue = parseReg(cleanArg.substr(1, cleanArg.size()-2));
      break;
    }
    case '>': {
      op += ARG(numArg, MANAGE_OPTIONS(ADDR) );
      istringstream(cleanArg.substr(1, cleanArg.size()-1)) >> argValue;
      break;
    }
    default:
      throw WrongArgumentException("Invalid Argument: " + cleanArg);
  }
  return argValue;
}

#undef MANAGE_OPTIONS

inline void
FunctionRecord::detectArgsOptions(const string& arg, string& cleanArg,
                                  int& options)
                throw(WrongArgumentException) {
  // the 6th bit just tells us that it's going to be an arg with options
  // It will be then filtered
  if (arg[0] == '+') {
    options = (1 << 5) + 0;
    cleanArg = arg.substr(1,arg.size()-1);
  } else if (arg[0] == '-') {
    options = (1 << 5) + 1;
    cleanArg = arg.substr(1,arg.size()-1);
  } else if (arg[arg.size()-1] == '+') {
    options = (1 << 5) + 2;
    cleanArg = arg.substr(0,arg.size()-2);
  } else if (arg[arg.size()-1] == '-') {
    options = (1 << 5) + 3;
    cleanArg = arg.substr(0,arg.size()-2);
  }

  // integrity check
  if ((cleanArg[0] == '+') || (cleanArg[0] == '-') ||
          (cleanArg[cleanArg.size()-1] == '+') ||
          (cleanArg[cleanArg.size()-1] == '-'))
  {
    throw WrongArgumentException("Ambiguos pre/post intement/decrement");
  }

  // Now that it's been cleaned by potentially
  if (cleanArg[0] == 'r') {
    options += (1 << 4);
    cleanArg = cleanArg.substr(1,cleanArg.size()-1);
  }
}

inline int
FunctionRecord::parseReg(const string& reg) throw(WrongArgumentException) {
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
