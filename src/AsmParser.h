/* 
 * File:   AsmParser.h
 * Author: ben
 *
 * Created on 27 agosto 2009, 14.26
 */

#ifndef _ASMPARSER_H
#define	_ASMPARSER_H

#include <string>
#include <map>

#include "FileHandler.h"
#include "../include/std_istructions.h"
#include "../include/asm_helpers.h"

using namespace std;

typedef map<string, unsigned int> Labels;
typedef map<string, int> Istructions;
typedef vector<int> Constants;
typedef vector< vector<string> > CodeLines;

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
  string fileContent;
  CodeLines lines;

  Bloat code;

  Labels labels;
  Constants consts;

  void preProcess();
  int processArgOp(int& op, const string& arg, const int& numArg);
  int parseReg(const string& reg);
public:
  AsmParser(const string& _fname) : fname(_fname), globalConsts(false) {
    TextLoader loader(fname.data());
    fileContent = loader.getTextFileContent();
  }
//  AsmParser(const AsmParser& orig);
  virtual ~AsmParser();

  void parse() throw(WrongArgumentException, WrongIstructionException);

  void assemble() {
    BinWriter writer((fname.substr(0,fname.size()-1) += "bin").data());
    writer.saveBinFileContent(code);
    printf("Assembled\n");
  }
};

#endif	/* _ASMPARSER_H */
