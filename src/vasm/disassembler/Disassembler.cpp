/*
 * Disassembler.cpp
 *
 *  Created on: 02/ago/2011
 *      Author: ben
 */

#include "Disassembler.h"

#include "parser_definitions.h"

#include <iostream>
#include <sstream>
#include <iterator>

void
Disassembler::printArg(const int32_t & typeArg, const int64_t & arg)
{
  cout << "       Type: ";
  cout.width(12);
  cout << ATypeSet.getItem(GET_ARG_TYPE(typeArg));
  cout.width(0);
  stringstream argString;
  switch (GET_ARG_TYPE(typeArg)) {
    case REG: {
      argString << "%" << RTypeSet.getItem(FILTER_PRE_POST(arg));
      break;
    }
    case REG_INDIR: {
      argString << "(%" << RTypeSet.getItem(FILTER_PRE_POST(arg)) << ")";
      break;
    }
    case MEM_INDIR: {
      argString << "( (%" << RTypeSet.getItem(FILTER_PRE_POST(arg)) << ") )";
      break;
    }
    case INDEXED: {
      argString << "(%" << RTypeSet.getItem(FILTER_PRE_POST(arg)) << ")["
            << RTypeSet.getItem(FILTER_PRE_POST(GET_INDEX_REG(arg))) <<"]";
      break;
    }
    case DISPLACED: {
      argString << RTypeSet.getItem(FILTER_PRE_POST(GET_BIG_DISPL(arg)))
            << "(%" << RTypeSet.getItem(FILTER_PRE_POST(arg)) << ")";
      break;
    }
    case INDX_DISP: {
      argString << RTypeSet.getItem(FILTER_PRE_POST(GET_BIG_DISPL(arg)))
            << "(%" << RTypeSet.getItem(FILTER_PRE_POST(arg)) << ")[%"
            << RTypeSet.getItem(FILTER_PRE_POST(GET_INDEX_DISPL(arg))) <<"]";
      break;
    }
    default: {
      argString << arg;
      break;
    }
  }
  cout << ", Arg: ";
  cout.width(10);
  cout << argString.str();
  cout.width(0);
  cout << ", Scale: " << STypeSet.getItem(GET_ARG_SCALE(typeArg));
  switch (GET_ARG_TYPE(typeArg)) {
    case REG:
    case REG_INDIR...INDX_DISP: {
      cout << ", Modifier: " << MTypeSet.getItem(GET_ARG_MOD(arg)) << endl;
      break;
    }
    default: {
      cout << endl;
      break;
    }
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
    cout << "Function: \"" << func.name << "\"" << endl << "Statements:" << endl;
    for(ListOfStmts::iterator stmtIt = func.stmts.begin();
        stmtIt != func.stmts.end(); stmtIt++)
    {
      asm_statement * stmt = *stmtIt;
      cout.width(4);
      cout.fill('0');
      cout << stmt->offset;
      cout.width(0);
      cout.fill(' ');
      cout << ":";
      if (stmt->getType() == ASM_LABEL_STATEMENT) {
        asm_label_statement * l_stmt = (asm_label_statement *) stmt;
        cout << "." << l_stmt->label << ": (position: ";
        if (l_stmt->isGlobal()) {
          cout << "global " << l_stmt->offset;
        } else {
          cout << "internal "
                << l_stmt->offset << ", total "
                << l_stmt->offset + func.functionOffset;
        }
        cout << ")" << endl;
      } else {
        bytecode.resize(stmt->getSize());
        Bloat::iterator it = bytecode.begin();
        stmt->emitCode(it);
        disassembleAndPrint(bytecode);
      }
    }
    cout << "Stack Locals:" << endl;
    this->printLocals(func.stackLocals, func.functionOffset);
    cout << "Unique Locals:" << endl;
    this->printLocals(func.uniqueLocals, func.functionOffset);

    cout << "End Function \"" << func.name << "\"" << endl << endl;
  }
}

inline void
Disassembler::printLocals(const ListOfDataStmts & locals, const size_t & funcOffset) const
{
  Bloat bytecode;
  for(ListOfDataStmts::const_iterator stmtIt = locals.begin();
      stmtIt != locals.end(); stmtIt++)
  {
    asm_data_statement * stmt = *stmtIt;
    if (stmt->getType() == ASM_LABEL_STATEMENT) {
      asm_label_statement * l_stmt = (asm_label_statement *) stmt;
      cout << "." << l_stmt->label << ": (position: ";
      if (l_stmt->isGlobal()) {
        cout << "global " << l_stmt->offset;
      } else {
        cout << "internal "
              << l_stmt->offset << ", total "
              << l_stmt->offset + funcOffset;
      }
      cout << ")" << endl;
    } else {
      bytecode.resize(stmt->getSize());
      Bloat::iterator it = bytecode.begin();
      stmt->emitCode(it);
      cout << "  ";
      copy(bytecode.begin(), bytecode.end(), ostream_iterator<int32_t>(cout, "  "));
      cout << endl;
    }
  }
}

const int64_t
Disassembler::fetchArg(const int32_t & typeArg, Bloat::const_iterator & codeIt,
    const Bloat::const_iterator & endIt)
{
  size_t numOfBytes = (GET_ARG_TYPE(typeArg) == IMMED) ? (1 << (GET_ARG_SCALE(typeArg))) : 4;
  if (codeIt > (endIt - numOfBytes)) {
    stringstream stream;
    stream << __PRETTY_FUNCTION__ << ": Overcame bytecode limit by "
            << codeIt - (endIt - numOfBytes) << " bytes." << endl;
    stream << "Pointer position: " << hex << reinterpret_cast<uint64_t>(&*codeIt)
            << " Ending position: " << hex << reinterpret_cast<uint64_t>(&*endIt)
            << " Number of bytes to read: " << numOfBytes << endl;
    throw WrongArgumentException(stream.str());
  }
  switch (numOfBytes) {
    case 1: {
      return (int64_t) *codeIt++;
    }
    case 2: {
      return (((int64_t) *codeIt++) + (((int64_t) *codeIt++) << 8));
    }
    case 4: {
      return DEAL_QWORD_FROM_SWORDS(codeIt);
//      return (((int64_t) *codeIt++) + (((int64_t) *codeIt++) << 8)
//          + (((int64_t) *codeIt++) << 16) + (((int64_t) *codeIt++) << 24));
    }
    case 8: {
      return (((int64_t) *codeIt++) + (((int64_t) *codeIt++) << 8)
          + (((int64_t) *codeIt++) << 16) + (((int64_t) *codeIt++) << 24)
          + (((int64_t) *codeIt++) << 32) + (((int64_t) *codeIt++) << 40)
          + (((int64_t) *codeIt++) << 48) + (((int64_t) *codeIt++) << 56));
    }
    default:
      throw WrongArgumentException(string(__PRETTY_FUNCTION__) + ": Unknown byte scale");
  }
}

void
Disassembler::disassembleAndPrint(const Bloat & bytecode)
{
  const Bloat::const_iterator & endIt = bytecode.end();
  for(Bloat::const_iterator codeIt = bytecode.begin(); codeIt < endIt;)
  {
    const int32_t instr = DEAL_QWORD_FROM_SWORDS(codeIt);
    try {
      if (instr) {
        switch (GET_NUM_ARGS(instr)) {
          case 0:
            cout << " " << ISet.getInstr(instr) << endl;
            break;
          case 1: {
            const int32_t typeArg1 = GET_ARG_1(instr);
            const int32_t polishedInstr = instr - ARG_1(typeArg1);
            cout << " " << ISet.getInstr(polishedInstr) << endl;
            printArg(typeArg1, this->fetchArg(typeArg1, codeIt, endIt));
            break;
          }
          case 2: {
            const int32_t typeArg1 = GET_ARG_1(instr);
            const int32_t typeArg2 = GET_ARG_2(instr);
            const int32_t polishedInstr = instr - ARG_1(typeArg1) - ARG_2(typeArg2);
            cout << " " << ISet.getInstr(polishedInstr) << endl;
            printArg(typeArg1, this->fetchArg(typeArg1, codeIt, endIt));
            printArg(typeArg2, this->fetchArg(typeArg2, codeIt, endIt));
            break;
          }
          case 3: {
            const int32_t typeArg1 = GET_ARG_1(instr);
            const int32_t typeArg2 = GET_ARG_2(instr);
            const int32_t typeArg3 = GET_ARG_3(instr);
            const int32_t polishedInstr = instr - ARG_1(typeArg1) - ARG_2(typeArg2)
                                            - ARG_3(typeArg3);
            cout << " " << ISet.getInstr(polishedInstr) << endl;
            printArg(typeArg1, this->fetchArg(typeArg1, codeIt, endIt));
            printArg(typeArg2, this->fetchArg(typeArg2, codeIt, endIt));
            printArg(typeArg3, this->fetchArg(typeArg3, codeIt, endIt));
            break;
          }
          default:
            throw WrongInstructionException(string(__PRETTY_FUNCTION__)
                + ": Impossible block reached!");
        }
      }
    } catch (const WrongInstructionException & e) {
      cout << "Skip instr: " << e.what() << endl;
    }
  }
}
