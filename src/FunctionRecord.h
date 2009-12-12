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

class FunctionRecord {
public:
  FunctionRecord(const string& _name, const Labels& _globalLabels) :
        name(_name), globalLabels(_globalLabels) { }
//  FunctionRecord(const FunctionRecord& orig);
//  virtual ~FunctionRecord();
private:
  const string name;

  Labels localLabels;

  const Labels& globalLabels;

  void storeLabel(const string& word, const int& bytePos)
          throw(DuplicateLabelException);

  void lineParsingKernel( const CodeLines::const_iterator& line, Bloat& code)
          throw(WrongArgumentException, WrongIstructionException);
  int processArgOp(int& op, const string& arg, const int& numArg);
  int parseReg(const string& reg);
};

#endif	/* _FUNCTIONRECORD_H */

