/*
 * AsmChecker.cpp
 *
 *  Created on: 09/feb/2013
 *      Author: ben
 */

#include "AsmChecker.h"

#include "../IncludesTree.h"
#include "parser_definitions.h"

#include <sstream>

void
AsmChecker::checkArgs(const asm_instruction_statement & stmt)
{
  // Check size
  size_t instrNumOfArgs = (stmt.instruction >> 30 ) & 3;
  if (instrNumOfArgs != stmt.args.size()) {
    std::stringstream stream;
    stream  << "Wrong number of arguments for instruction '"
              << ISet.getInstr(stmt.instruction)
              << "' (" << stmt.instruction << ") at position:" << std::endl
            << stmt.position.fileNode->printString()
              << " Line: " << stmt.position.first_line << std::endl
            << " - Arguments expected: " << instrNumOfArgs
              << ", passed: " << stmt.args.size() << std::endl;
    throw WrongArgumentException(stream.str());
  }

  // Check writable destination
  switch (stmt.instruction) {
    case NOT:
    case INCR:
    case DECR:
    case COMP2:
    case LSH:
    case RSH: {
      if (stmt.args[0]->type == IMMED) {
        std::stringstream stream;
        stream  << "Expected non constant argument for unary instruction '"
                  << ISet.getInstr(stmt.instruction)
                  << "' (" << stmt.instruction << ") at position:" << std::endl
                << stmt.position.fileNode->printString()
                  << " Line: " << stmt.position.first_line << std::endl;
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
      if (stmt.args[1]->type == IMMED) {
        std::stringstream stream;
        stream  << "Expected non constant destination argument for binary "
                  << "instruction '" << ISet.getInstr(stmt.instruction)
                  << "' (" << stmt.instruction << ") at position:" << std::endl
                << stmt.position.fileNode->printString()
                  << " Line: " << stmt.position.first_line << std::endl;
        throw WrongArgumentException(stream.str());
      }
      break;
    }
    default:
      break;
  }
}

void
AsmChecker::checkArgs(const asm_function_call & stmt)
{
  if (stmt.args.size() < 1) {
    std::stringstream stream;
    stream  << "Not enough arguments for function call at position:" << std::endl
            << stmt.position.fileNode->printString()
              << " Line: " << stmt.position.first_line << std::endl
            << " - At least the function label needs to be specified " << std::endl;
    throw WrongArgumentException(stream.str());
  }
  if (stmt.args[0]->getType() != ASM_LABEL_ARG) {
    std::stringstream stream;
    stream  << "Expected label as first argument of function call at position:"
              << std::endl
            << stmt.position.fileNode->printString()
              << " Line: " << stmt.position.first_line << std::endl;
    throw WrongArgumentException(stream.str());
  }
}

void
AsmChecker::checkArgs(const asm_return_statement & stmt)
{
  if (stmt.args.size() > 1) {
    std::stringstream stream;
    stream  << "Too many arguments for return statement at:" << std::endl
            << stmt.position.fileNode->printString()
              << " Line: " << stmt.position.first_line << std::endl
            << " - At most, you should specify a returned value" << std::endl;
    throw WrongArgumentException(stream.str());
  }
}

void
AsmChecker::checkCallParameters(const asm_function_call & stmt,
    const std::deque<asm_function *> & functions)
{
  const std::vector<asm_arg *> & args = stmt.args;
  const std::string & f_name = ((const asm_label_arg *)args[0])->label;
  for(size_t funcNum = 0; funcNum < functions.size(); funcNum++)
  {
    const asm_function & func = *(functions[funcNum]);
    if (func.name == f_name) {
      if (func.parameters.size() != (args.size() -1)) {
        std::stringstream stream;
        stream  << "Not enough arguments for function call at:"
                << std::endl << stmt.position.fileNode->printString()
                  << " Line: " << stmt.position.first_line << std::endl
                << " - Function '" << f_name << "' requires "
                  << func.parameters.size() << " parameters, while "
                  << (args.size() -1) << " were provided." << std::endl;
        throw WrongArgumentException(stream.str());
      }
      break;
    }
  }
}

void
AsmChecker::checkInstructions(const asm_program & prog, const bool & usingTemps)
{
  bool error = false;
  for(asm_function * func : prog.functions)
  {
    for(const asm_statement * stmt : func->stmts)
    {
      if (stmt->isInstruction()) {
        try {
          switch (stmt->getType()) {
            case ASM_RETURN_STATEMENT: {
              AsmChecker::checkArgs(*(const asm_return_statement *)stmt);
              break;
            }
            case ASM_FUNCTION_CALL: {
              const asm_function_call * f_stmt = (const asm_function_call *) stmt;
              AsmChecker::checkArgs(*f_stmt);

              if (usingTemps && !f_stmt->external) {
                AsmChecker::checkCallParameters(*f_stmt, prog.functions);
              }
              break;
            }
            default: {
              AsmChecker::checkArgs(*(const asm_instruction_statement *)stmt);
              break;
            }
          }
        } catch (const WrongArgumentException & e) {
          fprintf(stderr, "ERROR: in instruction!\n%s\n", e.what());
          error = true;
        }
      }
    }
  }
  if (error) {
    throw WrongArgumentException("Errors in instructions definition");
  }
}

bool
AsmChecker::ensureTempsUsage(const asm_function & func, const bool & used)
{
  bool error = false;
  for(const asm_statement * stmt : func.stmts)
  {
    if (stmt->getType() == ASM_INSTRUCTION_STATEMENT) {
      try {
        ((const asm_instruction_statement *)stmt)->ensureTempsUsage(used);
      } catch (const WrongArgumentException & e) {
        fprintf(stderr, "ERROR: in instruction!\n%s\n", e.what());
        error = true;
      }
    }
  }
  return error;
}

void
AsmChecker::ensureTempsUsage(const asm_program & prog, const bool & used)
{
  bool error = false;
  for(asm_function * func : prog.functions)
  {
    error |= AsmChecker::ensureTempsUsage(*func, used);
  }
  if (error) {
    throw WrongArgumentException(used
        ? "Explicit registers were found" : "Temporaries were found");
  }
}

