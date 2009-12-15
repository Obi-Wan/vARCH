/* 
 * File:   CodeOrganizer.h
 * Author: ben
 *
 * Created on 12 dicembre 2009, 15.40
 */

#ifndef _CODEORGANIZER_H
#define	_CODEORGANIZER_H

#include "FunctionRecord.h"

class CodeOrganizer {
public:
  CodeOrganizer() : tempFunc(NULL), main(NULL), global(false) { }
//  CodeOrganizer(const CodeOrganizer& orig);
//  virtual ~CodeOrganizer();

  void addLine(const CodeLines::value_type & ) throw(DuplicateLabelException);

  void assignGlobalSymbols();
  void assemble(Bloat& code);

  void storeLabel(const string& word, const unsigned int& bytePos,
                  const unsigned int& lineNum, const int& offset = 0)
          throw(DuplicateLabelException);

  const Labels& getLabels() const { return labels; }

  void parseFunctions();
private:
  FunctionRecord * tempFunc;
  FunctionRecord * main;
  Functions functions;

  Labels labels;
  Constants consts;
  CodeLines globals;

  bool global;

  void closeCurrentFunction(const int& lineNum);
};

#endif	/* _CODEORGANIZER_H */

