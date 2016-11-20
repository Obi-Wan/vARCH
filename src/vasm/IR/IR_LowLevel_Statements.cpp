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

void
asm_instruction_statement::ensureTempsUsage(const bool & used) const
{
  for(asm_arg * arg_g : args)
  {
    if (arg_g->getType() == ASM_IMMEDIATE_ARG)
    {
      const asm_immediate_arg * arg = (const asm_immediate_arg *) arg_g;
      if (!(arg->type == IMMED || arg->type == DIRECT) && (used ^ arg->isTemp))
      {
        std::stringstream stream;
        if (used) {
          stream << "Found an explicit register when compiling for temporaries";
        } else {
          stream << "Found a temporary when not compiling for temporaries,";
        }
        stream  << " as argument of instruction '" << ISet.getInstr(instruction)
                  << "' (" << instruction << ") at position:" << std::endl
                << position.fileNode->printString()
                  << " Line: " << position.first_line << std::endl;
        throw WrongArgumentException(stream.str());
      }
    }
  }
}

void
asm_instruction_statement::emitArgs(Bloat::iterator & position)
{
  for (asm_arg * arg : args) { arg->emitCode(position); }
}

void
asm_function_call::importParameters(const ListOfParams & params)
{
  parameters.clear();
  for(asm_function_param * par : params) { parameters.push_back(par); }
}

void
asm_int_keyword_statement::emitCode(Bloat::iterator & codeIt) {
  switch (scale) {
    case BYTE1: {
      *(codeIt++) = integer & BWORD;
      break;
    }
    case BYTE2: {
      *(codeIt++) = EXTRACT_LOWER__BWORD_FROM_HWORD(integer);
      *(codeIt++) = EXTRACT_HIGHER_BWORD_FROM_HWORD(integer);
      break;
    }
    case BYTE4: {
      const int8_t number[4] = DEAL_BWORDS_FROM_SWORD(integer);
      for(size_t count = 0; count < 4; count++) {
        *(codeIt++) = number[count];
      }
      break;
    }
    case BYTE8: {
      const int8_t number[8] = DEAL_BWORDS_FROM_DWORD(integer);
      for(size_t count = 0; count < 8; count++) {
        *(codeIt++) = number[count];
      }
      break;
    }
    default: {
      std::stringstream stream;
      stream << __FUNCTION__ << ": Wrong Number scale! I was expecting "
            << "valid values for the enumeration 'ScaleOfArgument', but "
            << "got: " << scale << std::endl;
      throw WrongArgumentException(stream.str());
    }
  }
}

