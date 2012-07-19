/*
 * ErrorReporter.h
 *
 *  Created on: 20/lug/2012
 *      Author: ben
 */

#ifndef ERRORREPORTER_H_
#define ERRORREPORTER_H_

#include "exceptions.h"
#include "IR/IR_LowLevel_Defs.h"
#include "IncludesTree.h"

#include <string>
#include <list>

using namespace std;

class ErrorReporter {
  list<string> listOfErrMsgs;
public:
  void addErrorMsg(const string && err);
  void addErrorMsg(const YYLTYPE & pos, const BasicException & e);

  static void printError(const YYLTYPE & pos, const BasicException & e);

  bool hasErrors() const { return !(this->listOfErrMsgs.empty()); }

  void throwError(BasicException && e);
};

#endif /* ERRORREPORTER_H_ */
