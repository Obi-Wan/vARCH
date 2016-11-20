/* 
 * File:   asm_program.cpp
 * Author: ben
 * 
 * Created on 5 agosto 2010, 21.05
 */

#include "asm-program.h"

#include "../FileHandler.h"

asm_program::~asm_program()
{
  for(asm_function * func : functions) { delete func; }
  for(asm_data_statement * stmt : shared_vars) { delete stmt; }
  for(asm_data_statement * stmt : constants) { delete stmt; }
}

void
asm_program::assemble(const std::string & outputName)
{
  Bloat bytecode;
  const size_t tot_size = getFunciontsTotalSize()
      + getSharedVarsTotalSize() + getConstantsTotalSize();

  bytecode.resize(tot_size, 0);
  Bloat::iterator pos = bytecode.begin();

  for(asm_function * func : functions)
  {
    for(asm_statement * stmt : func->stmts) { stmt->emitCode(pos); }
    pos += func->getPaddingSize();
  }
  for (asm_data_statement * stmt : shared_vars) { stmt->emitCode(pos); }
  for (asm_data_statement * stmt : constants) { stmt->emitCode(pos); }

  InfoPrintf(("Size of the generated file: %5u bytes\n",
              (uint32_t)bytecode.size() ));

  BinWriter writer(outputName.c_str());
  writer.saveBinFileContent(bytecode);
}

void
asm_program::emitDebugSymbols(const std::string & outputName) const
{
  TextWriter writer(outputName);
  writer << "GLOBAL_SYMBOLS" << std::endl
          << globalSymbols.emitDebugSymbols()
          << "END" << std::endl << std::endl;

  for(asm_function * func : functions)
  {
    writer << "FUNCTION \"" << func->name << "\"" << std::endl
            << func->localSymbols.emitDebugSymbols()
            << "END" << std::endl << std::endl;
  }
}

void
asm_program::emitXMLDebugSymbols(const std::string & outputName) const
{
  TextWriter writer(outputName);
  writer << "<global_symbols>" << std::endl
          << globalSymbols.emitXMLDebugSymbols()
          << "</global_symbols>" << std::endl;

  for(asm_function * func : functions)
  {
    writer << "<function name=\"" << func->name << "\">" << std::endl
            << func->localSymbols.emitXMLDebugSymbols()
            << "</function>" << std::endl;
  }
}


