/* 
 * File:   AsmParser.h
 * Author: ben
 *
 * Created on 27 agosto 2009, 14.26
 */

#ifndef _ASMPARSER_H
#define	_ASMPARSER_H

#include "FileHandler.h"
#include "../include/std_istructions.h"
#include "../include/asm_helpers.h"
#include "../include/parser_definitions.h"

class AsmParser {

  enum ConstsType {
    INT,
    LONG,
    CHAR,
    STRING,
    GLOBAL,
  };
  ConstsType getConstType(const string& type) const;
  bool globalConsts;

  const string fname;
  CodeLines lines;
  CodeLines linesIntermed;

  Bloat code;

  Labels labels;
  Constants consts;

  void preParser(const string& fileContent);
  void synthaxParser();

  void preProcess();
  void storeLabel(const string& word, const int& bytePos)
          throw(DuplicateLabelException);
  int processArgOp(int& op, const string& arg, const int& numArg);
  int parseReg(const string& reg);

  void lineParsingKernel( const CodeLines::const_iterator& line, Bloat& code)
          throw(WrongArgumentException, WrongIstructionException);
public:
  AsmParser(const string& _fname);
//  AsmParser(const AsmParser& orig);
//  virtual ~AsmParser();

  void parse() throw(WrongArgumentException, WrongIstructionException);

  void assemble() {
    BinWriter writer((fname.substr(0,fname.size()-1) += "bin").data());
    writer.saveBinFileContent(code);
    printf("Assembled\n");
  }
};

#endif	/* _ASMPARSER_H */

