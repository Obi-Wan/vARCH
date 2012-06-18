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
  cout << ", Modifier: ";;
  cout.width();
  switch (typeArg) {
    case REG:
    case REG_INDIR:
    case MEM_INDIR:
    case INDEXED:
    case DISPLACED:
    case INDX_DISP:
      cout << ", Arg: " << RTypeSet.getItem(arg) << endl;
      break;
    default:
      cout << ", Arg: " << arg << endl;
      break;
  }
}

void
Disassembler::disassembleProgram(asm_program & prog)
{
  Bloat bytecode;
  bytecode.reserve(5);
  for(size_t f_index = 0; f_index < prog.functions.size(); f_index++)
  {
    asm_function & func = *prog.functions[f_index];
    cout << "Function: \"" << func.name << "\"" << endl;
    for(ListOfStmts::iterator stmtIt = func.stmts.begin();
        stmtIt != func.stmts.end(); stmtIt++)
    {
      asm_statement * stmt = *stmtIt;
      if (stmt->getType() == ASM_LABEL_STATEMENT) {
        asm_label_statement * l_stmt = (asm_label_statement *) stmt;
        cout << "." << l_stmt->label << ": (position: internal "
              << l_stmt->offset << ", total "
              << l_stmt->offset + func.functionOffset << ")"  << endl;
      } else {
        bytecode.resize(stmt->getSize());
        Bloat::iterator it = bytecode.begin();
        stmt->emitCode(it);
        disassembleAndPrint(bytecode);
      }
    }
    cout << "End Function \"" << func.name << "\"" << endl << endl;
  }
}

void
Disassembler::disassembleAndPrint(const Bloat & bytecode)
{
  for(Bloat::const_iterator codeIt = bytecode.begin();codeIt != bytecode.end();)
  {
    const int32_t & instr = *(codeIt++);
    try {
    if (instr) {
      switch ((instr >> 30 ) & 3) {
        case 0:
          cout << "  " << ISet.getInstr(instr) << endl;
          break;
        case 1: {
          const int typeArg1 = GET_ARG_1(instr);
          const int polishedInstr = instr - ARG_1(typeArg1);
          cout << "  " << ISet.getInstr(polishedInstr) << endl;
          printArg(typeArg1, *codeIt++);
          break;
        }
        case 2: {
          const int typeArg1 = GET_ARG_1(instr);
          const int typeArg2 = GET_ARG_2(instr);
          const int polishedInstr = instr - ARG_1(typeArg1) - ARG_2(typeArg2);
          cout << "  " << ISet.getInstr(polishedInstr) << endl;
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
          cout << "  " << ISet.getInstr(polishedInstr) << endl;
          printArg(typeArg1, *codeIt++);
          printArg(typeArg2, *codeIt++);
          printArg(typeArg3, *codeIt++);
          break;
        }
        default:
          break;
      }
    }
    } catch (const WrongInstructionException & e) {
      cout << "Skip instr" << endl;
    }
  }
}
