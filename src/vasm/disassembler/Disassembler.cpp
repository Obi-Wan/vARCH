/*
 * Disassembler.cpp
 *
 *  Created on: 02/ago/2011
 *      Author: ben
 */

#include "Disassembler.h"

#include "parser_definitions.h"

#include "IR/IR_LowLevel_ParserHelpers.h"

#include <iostream>
#include <sstream>
#include <iterator>

void
Disassembler::disassembleBytecode(asm_program & prog, const int8_t * data,
    const size_t & size, const SymbolsTempLookup<> & lookup_func,
    const OffsetTempLookup<> & lookup_rel)
{
  for(size_t indx = 0; indx < size;)
  {
    const std::string & func_name = lookup_func.getString(indx);
    DebugPrintf(("Loading function at: 0x%04lx (%04lu), '%s'\n", indx, indx,
        func_name.c_str()));

    asm_function * func = new asm_function(YYLTYPE(), func_name);
    const size_t & func_size = prog.globalSymbols.getStmt(func_name)->size;
    func->stmts.push_back(prog.globalSymbols.getStmt(func_name));

    const int8_t * func_data = data + indx;
    const int8_t * code_p_end = func_data + func_size;

    for(const int8_t * code_p = func_data; code_p < code_p_end;)
    {
      uint32_t instr;
      std::vector<TypeOfArgument> type_args;
      std::vector<ScaleOfArgument> scale_args;

      decodeInstruction(code_p, instr, type_args, scale_args);

      asm_instruction_statement * stmt = InstructionsHandler::getStmt(YYLTYPE(), instr);

      DebugPrintf(("STMT instruction: '%s'\n",
          ISet.getInstr(stmt->instruction).c_str() ));

      for (size_t arg_num = 0; arg_num < type_args.size(); arg_num++)
      {
        const TypeOfArgument & type_arg = type_args[arg_num];
        const ScaleOfArgument & scale_arg = scale_args[arg_num];

        const size_t arg_offset = size_t(code_p - data);

        const ArgumentValue && arg = this->fetchArg(type_arg, scale_arg, code_p, code_p_end);

        asm_arg * arg_struct = this->decodeArgument(type_arg, scale_arg, arg);

        DebugPrintf(("  - Arg type: '%s', scale: '%s', offset 0x%04lx (%04lu)\n",
            ATypeSet.getItem(type_arg).c_str(),
            STypeSet.getItem(scale_arg).c_str(), arg_offset, arg_offset));

        if (lookup_rel.isOffset(arg_offset))
        {
          const std::string & label_name = lookup_rel.getString(arg_offset);
          asm_label_arg * label_ref = new asm_label_arg(
              *(asm_immediate_arg *)arg_struct, label_name);

          delete arg_struct;
          arg_struct = label_ref;

          ArgLabelRecord * tempRecord = new ArgLabelRecord();
          tempRecord->arg = (asm_label_arg *) arg_struct;
          tempRecord->parent = stmt;
          func->refs.push_back(tempRecord);

          DebugPrintf(("Found reference to label: offset 0x%04lx, '%s', type %s\n",
              arg_offset, label_name.c_str(), ATypeSet.getItem(arg_struct->type).c_str()));
        }

        stmt->args.push_back(arg_struct);
      }

      func->stmts.push_back(stmt);
    }

    prog.functions.push_back(func);

    indx += func_size;
  }
}

asm_arg *
Disassembler::decodeArgument(const TypeOfArgument & p_arg_type,
    const ScaleOfArgument & p_arg_scale, const ArgumentValue & arg)
{
  if (p_arg_type == IMMED)
  {
    return new asm_immediate_arg(YYLTYPE(), arg.immed, p_arg_type, p_arg_scale,
        REG_NO_ACTION, false);
  }
  else if (p_arg_type == DIRECT)
  {
    return new asm_immediate_arg(YYLTYPE(), arg.direct, p_arg_type, p_arg_scale,
        REG_NO_ACTION, false);
  }
  else
  {
    asm_immediate_arg * arg_out = new asm_immediate_arg(YYLTYPE());
    arg_out->type = p_arg_type;
    arg_out->scale = p_arg_scale;

    switch (p_arg_type) {
      case REG: {
        arg_out->content.regNum = arg.reg.reg_id;
        arg_out->regModType = arg.reg.reg_mod;
        break;
      }
      case REG_INDIR: {
        arg_out->content.regNum = arg.reg_indir.reg_id;
        arg_out->regModType = arg.reg_indir.reg_mod;
        break;
      }
      case MEM_INDIR: {
        arg_out->content.regNum = arg.mem_indir.reg_id;
        arg_out->regModType = arg.mem_indir.reg_mod;
        break;
      }
      case DISPLACED: {
        arg_out->content.regNum = arg.displaced.reg_id;
        arg_out->displacement = arg.displaced.disp;
        arg_out->regModType = arg.displaced.reg_mod;
        break;
      }
      case INDEXED: {
        arg_out->index = arg.indexed.indx_id;
        arg_out->content.regNum = arg.indexed.reg_id;
        arg_out->regModType = arg.indexed.indx_mod;
        break;
      }
      case INDX_DISP: {
        arg_out->displacement = arg.indx_disp.disp;
        arg_out->index = arg.indx_disp.indx_id;
        arg_out->content.regNum = arg.indx_disp.reg_id;
        arg_out->regModType = arg.indx_disp.indx_mod;
        break;
      }
      default: { break; }
    }

    return arg_out;
  }
}

void
Disassembler::printArg(const TypeOfArgument & typeArg,
    const ScaleOfArgument & scaleArg, const ArgumentValue & arg)
{
  std::cout << "       Type: ";
  std::cout.width(12);
  std::cout << ATypeSet.getItem(typeArg);
  std::cout.width(0);
  std::stringstream argString;
  switch (typeArg) {
    case IMMED: {
      argString << arg.immed;
      break;
    }
    case DIRECT: {
      argString << arg.direct;
      break;
    }
    case REG: {
      argString << "%" << RTypeSet.getItem(arg.reg.reg_id);
      break;
    }
    case REG_INDIR: {
      argString << "(%" << RTypeSet.getItem(arg.reg_indir.reg_id) << ")";
      break;
    }
    case MEM_INDIR: {
      argString << "( (%" << RTypeSet.getItem(arg.mem_indir.reg_id) << ") )";
      break;
    }
    case INDEXED: {
      argString << "(%" << RTypeSet.getItem(arg.indexed.reg_id) << ")["
            << RTypeSet.getItem(arg.indexed.indx_id) <<"]";
      break;
    }
    case DISPLACED: {
      argString << arg.displaced.disp
            << "(%" << RTypeSet.getItem(arg.displaced.reg_id) << ")";
      break;
    }
    case INDX_DISP: {
      argString << arg.indx_disp.disp
            << "(%" << RTypeSet.getItem(arg.indx_disp.reg_id) << ")[%"
            << RTypeSet.getItem(arg.indx_disp.indx_id) <<"]";
      break;
    }
  }
  std::cout << ", Arg: ";
  std::cout.width(10);
  std::cout << argString.str();
  std::cout.width(0);
  std::cout << ", Scale: " << STypeSet.getItem(scaleArg);
  switch (typeArg) {
    case REG: {
      std::cout << ", Reg Mod: " << MTypeSet.getItem(arg.reg.reg_mod) << std::endl;
      break;
    }
    case REG_INDIR: {
      std::cout << ", Reg Mod: " << MTypeSet.getItem(arg.reg_indir.reg_mod) << std::endl;
      break;
    }
    case MEM_INDIR: {
      std::cout << ", Reg Mod: " << MTypeSet.getItem(arg.mem_indir.reg_mod) << std::endl;
      break;
    }
    case DISPLACED: {
      std::cout << ", Reg Mod: " << MTypeSet.getItem(arg.displaced.reg_mod) << std::endl;
      break;
    }
    case INDEXED: {
      std::cout << ", Reg Mod: " << MTypeSet.getItem(arg.indexed.indx_mod) << std::endl;
      break;
    }
    case INDX_DISP: {
      std::cout << ", Reg Mod: " << MTypeSet.getItem(arg.indx_disp.indx_mod) << std::endl;
      break;
    }
    default: {
      std::cout << std::endl;
      break;
    }
  }
}

void
Disassembler::disassembleProgram(const asm_program & prog)
{
  Bloat bytecode;
  bytecode.reserve(5);
  for(const asm_function * func : prog.functions)
  {
    std::cout << "Function: \"" << func->name << "\"" << std::endl << "Statements:" << std::endl;
    for(asm_statement * stmt : func->stmts)
    {
      std::cout.width(4);
      std::cout.fill('0');
      std::cout << stmt->offset;
      std::cout.width(0);
      std::cout.fill(' ');
      std::cout << ":";
      if (stmt->getType() == ASM_LABEL_STATEMENT) {
        asm_label_statement * l_stmt = (asm_label_statement *) stmt;
        std::cout << "." << l_stmt->label << ": (position: ";
        if (l_stmt->isGlobal()) {
          std::cout << "global " << l_stmt->offset;
        } else {
          std::cout << "internal "
                << l_stmt->offset << ", total "
                << l_stmt->offset + func->functionOffset;
        }
        std::cout << ")" << std::endl;
      } else {
        bytecode.resize(stmt->getSize());
        Bloat::iterator it = bytecode.begin();
        stmt->emitCode(it);
        disassembleAndPrint(bytecode);
      }
    }
    std::cout << "Stack Locals:" << std::endl;
    this->printLocals(func->stackLocals, func->functionOffset);

    std::cout << "End Function \"" << func->name << "\"" << std::endl << std::endl;
  }

  std::cout << "Shared Variables (size: " << prog.getSharedVarsTotalSize() << "):" << std::endl;
  this->printLocals(prog.shared_vars, 0);
  std::cout << "End Shared Variables" << std::endl << std::endl;

  std::cout << "Constants (size: " << prog.getConstantsTotalSize() << "):" << std::endl;
  this->printLocals(prog.constants, 0);
  std::cout << "End Constants" << std::endl << std::endl;
}

inline void
Disassembler::printLocals(const ListOfDataStmts & locals, const size_t & funcOffset) const
{
  Bloat bytecode;
  for(asm_data_statement * stmt : locals)
  {
    if (stmt->getType() == ASM_LABEL_STATEMENT) {
      const asm_label_statement * l_stmt = (const asm_label_statement *) stmt;
      std::cout << "." << l_stmt->label << ": (position: ";
      if (l_stmt->isGlobal()) {
        std::cout << "global " << l_stmt->offset;
      } else {
        std::cout << "internal "
              << l_stmt->offset << ", total "
              << l_stmt->offset + funcOffset;
      }
      std::cout << ")" << std::endl;
    } else {
      bytecode.resize(stmt->getSize());
      Bloat::iterator it = bytecode.begin();
      stmt->emitCode(it);
      for(const auto & num : bytecode)
      {
        std::cout << " ";
        std::cout.width(3);
        std::cout << uint32_t(num);
        std::cout.width(0);
      }
      std::cout << std::endl;
    }
  }
}

ArgumentValue
Disassembler::fetchArg(const TypeOfArgument & typeArg,
    const ScaleOfArgument & scaleArg, const int8_t *& codeIt,
    const int8_t * const endIt)
{
  size_t numOfBytes = (typeArg == IMMED) ? (1 << scaleArg) : 4;
  if (codeIt > (endIt - numOfBytes)) {
    std::stringstream stream;
    stream << __PRETTY_FUNCTION__ << ": Overcame bytecode limit by "
            << codeIt - (endIt - numOfBytes) << " bytes." << std::endl;
    stream << "Pointer position: " << std::hex << reinterpret_cast<uint64_t>(&*codeIt)
            << " Ending position: " << std::hex << reinterpret_cast<uint64_t>(&*endIt)
            << " Number of bytes to read: " << numOfBytes << std::endl;
    throw WrongArgumentException(stream.str());
  }

  switch (numOfBytes) {
    case 1:
    case 2:
    case 4: { break; }
//    case 8: {
//      return DEAL_DWORD_FROM_BWORDS<int64_t>(codeIt);
//    }
    default:
      throw WrongArgumentException(std::string(__PRETTY_FUNCTION__)
          + ": Unknown byte scale: " + std::to_string(numOfBytes));
  }

  ArgumentValue temp_arg;
  for(size_t num = 0; num < numOfBytes; num++)
  {
    temp_arg.bytes[num] = *codeIt++;
  }
  return temp_arg;
}

void
Disassembler::decodeInstruction(const int8_t *& data, uint32_t & instr,
    std::vector<TypeOfArgument> & type_args, std::vector<ScaleOfArgument> & scale_args)
{
  instr = DEAL_SWORD_FROM_BWORDS(data);

  for (size_t arg_num = 0; arg_num < GET_NUM_ARGS(instr); arg_num++)
  {
    const uint32_t arg = GET_ARG(arg_num, instr);
    const TypeOfArgument type_arg = (TypeOfArgument)GET_ARG_TYPE(arg);
    const ScaleOfArgument scale_arg = (ScaleOfArgument)GET_ARG_SCALE(arg);
    type_args.push_back(type_arg);
    scale_args.push_back(scale_arg);
    instr -= ARG(arg_num, arg);
  }
}

void
Disassembler::disassembleAndPrint(const Bloat & bytecode)
{
  const int8_t * code_p_end = &*bytecode.end();
  for(const int8_t * code_p = &*bytecode.begin(); code_p < code_p_end;)
  {
    uint32_t instr;
    std::vector<TypeOfArgument> type_args;
    std::vector<ScaleOfArgument> scale_args;

    decodeInstruction(code_p, instr, type_args, scale_args);

    std::cout << " " << ISet.getInstr(instr) << std::endl;
    for (size_t arg_num = 0; arg_num < type_args.size(); arg_num++)
    {
      const TypeOfArgument & type_arg = type_args[arg_num];
      const ScaleOfArgument & scale_arg = scale_args[arg_num];
      printArg(type_arg, scale_arg, this->fetchArg(type_arg, scale_arg, code_p, code_p_end));
    }
  }
}
