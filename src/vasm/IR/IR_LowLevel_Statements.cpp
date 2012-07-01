/*
 * IR_LowLevel_Statements.cpp
 *
 *  Created on: 29/lug/2011
 *      Author: ben
 */

#include "IR_LowLevel_Statements.h"
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
              << ISet.getInstr(instruction)
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
      if (args[0]->type == IMMED) {
        stringstream stream;
        stream  << "Expected non constant argument for unary instruction '"
                  << ISet.getInstr(instruction)
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
      if (args[1]->type == IMMED) {
        stringstream stream;
        stream  << "Expected non constant destination argument for binary "
                  << "instruction '" << ISet.getInstr(instruction)
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

void
asm_instruction_statement::ensureTempsUsage(const bool & used) const
{
  for(size_t numArg = 0; numArg < args.size(); numArg++)
  {
    if (args[numArg]->getType() == ASM_IMMEDIATE_ARG)
    {
      const asm_immediate_arg * arg = (const asm_immediate_arg *) args[numArg];
      if (!(arg->type == IMMED || arg->type == DIRECT) && (used ^ arg->isTemp))
      {
        stringstream stream;
        if (used) {
          stream << "Found an explicit register when compiling for temporaries";
        } else {
          stream << "Found a temporary when not compiling for temporaries,";
        }
        stream  << " as argument of instruction '" << ISet.getInstr(instruction)
                  << "' (" << instruction << ") at position:" << endl
                << position.fileNode->printString()
                  << " Line: " << position.first_line << endl;
        throw WrongArgumentException(stream.str());
      }
    }
  }
}


void
asm_instruction_statement::emitArgs(Bloat::iterator & position) {
  for (size_t argNum = 0; argNum < args.size(); argNum++) {
    this->emitArg(args[argNum], position);
  }
}

void
asm_instruction_statement::emitArg(const asm_arg * const arg, Bloat::iterator & position)
{
  const int64_t tempArg = arg->getCode();
  switch (arg->scale) {
    case BYTE1: {
      *(position++) = (tempArg & SWORD);
      break;
    }
    case BYTE2: {
      *(position++) = EXTRACT_LOWER__SWORD_FROM_DWORD(tempArg);
      *(position++) = EXTRACT_HIGHER_SWORD_FROM_DWORD(tempArg);
      break;
    }
    case BYTE4: {
      const int8_t chunks[4] = DEAL_SWORDS_FROM_QWORD(tempArg);
      for(size_t count = 0; count < 4; count++) {
        *(position++) = chunks[count];
      }
      break;
    }
    case BYTE8: {
      // XXX Be careful that right now there is no correct handling of this
      const int8_t chunks[8] = DEAL_SWORDS_FROM_OWORD(tempArg);
      for(size_t count = 0; count < 8; count++) {
        *(position++) = chunks[count];
      }
      break;
    }
    default:
      throw WrongArgumentException(string(__FUNCTION__) + ": no such argument scale");
  }
}

void
asm_function_call::importParameters(const ListOfParams & params)
{
  parameters.clear();
  for(ListOfParams::const_iterator par = params.begin(); par != params.end(); par++)
  {
    parameters.push_back(*par);
  }
//  copy(params.begin(), params.end(), parameters.begin());
}

void
asm_function_call::checkArgs() const
{
  if (args.size() < 1) {
    stringstream stream;
    stream  << "Not enough arguments for function call at position:" << endl
            << position.fileNode->printString()
              << " Line: " << position.first_line << endl
            << " - At least the function label needs to be specified " << endl;
    throw WrongArgumentException(stream.str());
  }
  if (args[0]->getType() != ASM_LABEL_ARG) {
    stringstream stream;
    stream  << "Expected label as first argument of function call at position:"
              << endl
            << position.fileNode->printString()
              << " Line: " << position.first_line << endl;
    throw WrongArgumentException(stream.str());
  }
}

void
asm_return_statement::checkArgs() const
{
  if (args.size() > 1) {
    stringstream stream;
    stream  << "Too many arguments for return statement at:" << endl
            << position.fileNode->printString()
              << " Line: " << position.first_line << endl
            << " - At most, you should specify a returned value" << endl;
    throw WrongArgumentException(stream.str());
  }
}

