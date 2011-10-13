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
public:
  AsmArgs(int _argc, char** _argv)
    : argc(_argc), argv(_argv), regAutoAlloc(false), regCoalesce(false) { }

  void parse() throw(WrongArgumentException);

  void printHelp() const throw();

  const string &getInputName() const throw() { return inputName; }
  const string &getOutputName() const throw() { return outputName; }
  const string &getDebugSymbolsName() const throw() { return debugSymbolsName; }
  const vector<string> &getIncludeDirs() const throw() { return includeDirs; }
  const uint32_t &getOptimizationLevel() const throw() { return optimLevel; }
  const bool &getRegAutoAlloc() const throw() { return regAutoAlloc; }
  const bool &getRegCoalesce() const throw() { return regCoalesce; }
};

#endif /* ASMARGS_H_ */
