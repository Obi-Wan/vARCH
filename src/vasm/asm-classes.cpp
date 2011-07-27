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
asm_instruction_statement::checkArgs() const
{
  // Check size
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

  // Check writable destination
  switch (instruction) {
    case NOT:
    case INCR:
    case DECR:
    case COMP2:
    case LSH:
    case RSH: {
      if (args[0]->type == COST) {
        stringstream stream;
        stream  << "Expected non constant argument for unary instruction '"
                  << ISet.getIstr(instruction)
                  << "' (" << instruction << ") at position:" << endl
                << position.fileNode->printString()
                  << " Line: " << position.first_line << endl;
        throw WrongArgumentException(stream.str());
      }
      break;
    }
    case ADD:
    case MULT:
    case SUB:
    case DIV:
    case QUOT:
    case AND:
    case OR:
    case XOR:
    case GET: {
      if (args[1]->type == COST) {
        stringstream stream;
        stream  << "Expected non constant destination argument for binary "
                  << "instruction '" << ISet.getIstr(instruction)
                  << "' (" << instruction << ") at position:" << endl
                << position.fileNode->printString()
                  << " Line: " << position.first_line << endl;
        throw WrongArgumentException(stream.str());
      }
      break;
    }
    default:
      break;
  }
}
