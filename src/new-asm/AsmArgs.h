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

class AsmArgs {
  int argc;
  char** argv;

  string inputName;
  string outputName;
public:
  AsmArgs(int _argc, char** _argv) : argc(_argc), argv(_argv) { }

  void parse() throw(WrongArgumentException);

  void printHelp() const throw();

  const string &getInputName() const throw() { return inputName; }
  const string &getOutputName() const throw() { return outputName; }
};

#endif /* ASMARGS_H_ */
