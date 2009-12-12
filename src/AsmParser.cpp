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

AsmParser::AsmParser(const string& _fname) : fname(_fname), globalConsts(false) {
  TextLoader loader(fname.data());
  preParser(loader.getTextFileContent());
}

//AsmParser::AsmParser(const AsmParser& orig) { }

//AsmParser::~AsmParser() { }

inline void
AsmParser::preParser(const string& fileContent) {
  istringstream fileStr(fileContent);
  // Scans over the lines
  for(unsigned int lineNum = 0; !fileStr.eof(); lineNum++) {
    vector<string> lineOfCode;
    string word, line;
    
    getline(fileStr, line);
    // Don't store comments or blank lines
    if (line[0] == ';' || line.empty()) continue;

    istringstream lineStr(line);
    lineStr >> word;
    if (!word.empty()) {
      lineOfCode.push_back(word);
      // now if it's a constant we save it all together
      if ((word[0] == '.') && (word[word.size()-1] != ':') &&
              (getConstType(word) != GLOBAL)) {
        lineStr.ignore(256,'$');
        string constString;
        getline(lineStr, constString);
        lineOfCode.push_back(constString);
      // otherwise let's add it direcly without processing
      } else {
        for(; (lineStr >> word) && !word.empty(); ) {
          lineOfCode.push_back(word);
        }
      }
    }

    linesIntermed.push_back(CodeLines::value_type(lineNum,lineOfCode));
  }
  DebugPrintf(("preParser finished, code lines are:\n"));
  DebugPrintfCodeLines(linesIntermed, "lines2" );
}

inline void
AsmParser::synthaxParser() {
  //FIXME it's just useless now
}

inline void
AsmParser::storeLabel(const string& word, const int& bytePos)
            throw(DuplicateLabelException)
{
  string label = word.substr(1,word.size()-2);

  if (labels.find(label) != labels.end()) {
    throw DuplicateLabelException(
            "Label already defined here: " + labels.find(label)->second);
  }
  labels.insert(Labels::value_type(label, bytePos));
}

inline void
AsmParser::preProcess() {
  unsigned int bytePos = 0;
  for(CodeLines::const_iterator line = linesIntermed.begin(); line != linesIntermed.end(); line++ ) {
    const string& word = line->second[0];
    if (word[0] == '.') {
      if (word[word.size()-1] == ':') { // it's a label
        storeLabel(word, bytePos);
      } else { // it's a type marker
        if (!globalConsts) {
          if (getConstType(word) == GLOBAL) {
            globalConsts = true;
          } else {
            throw WrongIstructionException("No constants allowed here");
          }
        } else { /* Ok, we are processing constants */
          switch (getConstType(word)) {
            case INT: {
              istringstream argStr( line->second[1] );
              int value;
              argStr >> value;
              consts.push_back(value);
              bytePos++;
              break;
            }
            case CHAR:
            case STRING: {
              const string& temp = line->second[1];
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
      lines.push_back(CodeLines::value_type(line->first, line->second));
      bytePos += line->second.size();
    }
  }

  DebugPrintfMaps(Labels, labels, "labels");
  //DebugPrintfMaps(Constants, consts, "consts");

  printf("PreProcess Finished\n");
}

void
AsmParser::parse() throw(WrongArgumentException, WrongIstructionException) {
  preProcess();

  for(CodeLines::const_iterator line = lines.begin(); line != lines.end(); line++) {
    lineParsingKernel(line, code);
  }

  // pushing constants
  for (unsigned int i = 0; i < consts.size(); i++) {
    code.push_back(consts[i]);
  }

  printf("Parsing Finished\n");
}

inline void
AsmParser::lineParsingKernel( const CodeLines::const_iterator& line, Bloat& code)
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
AsmParser::parseReg(const string& reg) {
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