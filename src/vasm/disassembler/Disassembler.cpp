/*
 * Disassembler.cpp
 *
 *  Created on: 02/ago/2011
 *      Author: ben
 */

#include "Disassembler.h"

#include "parser_definitions.h"

#include <iostream>

void
Disassembler::printArg(const int & typeArg, const int & arg)
{
  cout << "       Type Arg: ";
  cout.width(12);
  cout << ATypeSet.getItem(typeArg & (0xF));
  cout.width(0);
  cout << ", Relative: " << boolalpha;
  cout.width(5);
  cout << ((typeArg & RELATIVE_ARG) == RELATIVE_ARG);
  cout.width();
  cout << ", Arg: " << arg << endl;
}

void
Disassembler::disassembleAndPrint(const Bloat & bytecode)
{
  for(Bloat::const_iterator codeIt = bytecode.begin(); codeIt != bytecode.end();)
  {
    const int32_t & instr = *(codeIt++);
    try {
    if (instr) {
      switch ((instr >> 30 ) & 3) {
        case 0:
          cout << "  " << ISet.getIstr(instr) << endl;
          break;
        case 1: {
          const int typeArg1 = GET_ARG_1(instr);
          const int polishedInstr = instr - ARG_1(typeArg1);
          cout << "  " << ISet.getIstr(polishedInstr) << endl;
          printArg(typeArg1, *codeIt++);
          break;
        }
        case 2: {
          const int typeArg1 = GET_ARG_1(instr);
          const int typeArg2 = GET_ARG_2(instr);
          const int polishedInstr = instr - ARG_1(typeArg1) - ARG_2(typeArg2);
          cout << "  " << ISet.getIstr(polishedInstr) << endl;
          printArg(typeArg1, *codeIt++);
          printArg(typeArg2, *codeIt++);
          break;
        }
        case 3: {
          const int typeArg1 = GET_ARG_1(instr);
          const int typeArg2 = GET_ARG_2(instr);
          const int typeArg3 = GET_ARG_3(instr);
          const int polishedInstr = instr - ARG_1(typeArg1) - ARG_2(typeArg2)
                                          - ARG_3(typeArg3);
          cout << "  " << ISet.getIstr(polishedInstr) << endl;
          printArg(typeArg1, *codeIt++);
          printArg(typeArg2, *codeIt++);
          printArg(typeArg3, *codeIt++);
          break;
        }
        default:
          break;
      }
    }
    } catch (const WrongIstructionException & e) {
      cout << "Skip instr" << endl;
    }
  }
}
