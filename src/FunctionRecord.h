/* 
 * File:   FunctionRecord.h
 * Author: ben
 *
 * Created on 10 dicembre 2009, 21.26
 */

#ifndef _FUNCTIONRECORD_H
#define	_FUNCTIONRECORD_H

#include "FileHandler.h"
#include "../include/asm_helpers.h"
#include "../include/std_istructions.h"
#include "../include/parser_definitions.h"
#include "../include/exceptions.h"
#include "../include/macros.h"

class FunctionRecord {
public:
  FunctionRecord(const string& _name, const Labels& _globalLabels) :
    name(_name), globalLabels(_globalLabels), bytes(0)
  {
    DebugPrintf(("creating record: %s\n", name.c_str()));
  }
//  FunctionRecord(const FunctionRecord& orig);
//  virtual ~FunctionRecord();
  void addLine(const CodeLines::value_type &line) {
    DebugPrintf(("Adding line to the function: %s\n", name.c_str()));
    linesIntermed.push_back(line);
    bytes += line.bytes;
  }
//  static MarkersType getMarkerType(const string& type);

  void preProcess() throw(WrongIstructionException, WrongArgumentException);
  void assemble();

  unsigned int getBytes() const { return bytes; }
  const string &getName() const { return name;  }
  const Bloat  &getCode() const { return code;  }
private:
  const string name;
  unsigned int bytes;

  Labels localLabels;
  const Labels& globalLabels;

  CodeLines linesIntermed;
  CodeLines lines;
  Constants consts;
  Bloat code;

  void storeLabel(const string& word, const int& bytePos, const int& lineNum)
          throw(DuplicateLabelException);

  void lineAssembleKernel( const CodeLines::const_iterator& line, Bloat& code,
                           int bytePos)
          throw(WrongArgumentException, WrongIstructionException);
  int processArgOp( int& op, const string& arg, const int& numArg,
                    const int& bytePos);
  int parseReg(const string& reg);
};

typedef vector<FunctionRecord *> Functions;

#endif	/* _FUNCTIONRECORD_H */

