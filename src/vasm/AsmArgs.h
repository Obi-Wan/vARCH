/*
 * AsmArgs.h
 *
 *  Created on: 20/mar/2011
 *      Author: ben
 */

#ifndef ASMARGS_H_
#define ASMARGS_H_

#include "macros.h"
#include "exceptions.h"

#include <vector>
using namespace std;

class AsmArgs {
  int argc;
  char** argv;

  string inputName;
  string outputName;
  string debugSymbolsName;
  vector<string> includeDirs;

  uint32_t optimLevel;

  bool regAutoAlloc;
  bool regCoalesce;

  bool onlyValidate;
  bool disassembleResult;
public:
  AsmArgs(int _argc, char** _argv)
    : argc(_argc), argv(_argv), optimLevel(2), regAutoAlloc(false)
    , regCoalesce(false), onlyValidate(false), disassembleResult(false)
  { }

  void parse() throw(WrongArgumentException);

  void printHelp() const throw();

  const string &getInputName() const throw() { return inputName; }
  const string &getOutputName() const throw() { return outputName; }
  const string &getDebugSymbolsName() const throw() { return debugSymbolsName; }
  const vector<string> &getIncludeDirs() const throw() { return includeDirs; }
  const uint32_t &getOptimizationLevel() const throw() { return optimLevel; }
  const bool &getRegAutoAlloc() const throw() { return regAutoAlloc; }
  const bool &getRegCoalesce() const throw() { return regCoalesce; }
  const bool &getOnlyValidate() const throw() { return onlyValidate; }
  const bool &getDisassembleResult() const throw() { return disassembleResult; }
};

#endif /* ASMARGS_H_ */
