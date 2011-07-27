/*
 * asm-classes.cpp
 *
 *  Created on: 20/mar/2011
 *      Author: ben
 */

#include "asm-classes.h"
#include "parser_definitions.h"
#include "IncludesTree.h"

#include <sstream>
using namespace std;

void
asm_instruction_statement::checkArgs() const {
  size_t instrNumOfArgs = (instruction >> 30 ) & 3;
  if (instrNumOfArgs != args.size()) {
    stringstream stream;
    stream  << "Wrong number of arguments for instruction '"
              << ISet.getIstr(instruction)
              << "' (" << instruction << ") at position:" << endl
            << position.fileNode->printString()
              << " Line: " << position.first_line << endl
            << " - Arguments expected: " << instrNumOfArgs
              << ", passed: " << args.size() << endl;
    throw WrongArgumentException(stream.str());
  }
}
