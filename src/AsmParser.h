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
#include "CodeOrganizer.h"

class AsmParser {

  const string fname;

  Bloat code;
  CodeOrganizer organizer;

  void addAllChunks(istringstream& inputLine, LineOfCode& outputLine);

  void preParser(const string& fileContent);
  void synthaxParser();

public:
  AsmParser(const string& _fname);
//  AsmParser(const AsmParser& orig);
//  virtual ~AsmParser();

  void parse() throw(WrongArgumentException, WrongIstructionException);
  void assemble();

  void flush() {
    BinWriter writer((fname.substr(0,fname.size()-1) += "bin").data());
    writer.saveBinFileContent(code);
    printf("Assembled\n");
  }
};

#endif	/* _ASMPARSER_H */

