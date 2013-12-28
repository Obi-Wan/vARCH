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
}

void
asm_program::assemble(const string & outputName)
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
asm_program::emitDebugSymbols(const string & outputName) const
{
  TextWriter writer(outputName);
  writer << "GLOBAL_SYMBOLS" << endl
          << globalSymbols.emitDebugSymbols()
          << "END" << endl << endl;

  for(asm_function * func : functions)
  {
    writer << "FUNCTION \"" << func->name << "\"" << endl
            << func->localSymbols.emitDebugSymbols()
            << "END" << endl << endl;
  }
}

void
asm_program::emitXMLDebugSymbols(const string & outputName) const
{
  TextWriter writer(outputName);
  writer << "<global_symbols>" << endl
          << globalSymbols.emitXMLDebugSymbols()
          << "</global_symbols>" << endl;

  for(asm_function * func : functions)
  {
    writer << "<function name=\"" << func->name << "\">" << endl
            << func->localSymbols.emitXMLDebugSymbols()
            << "</function>" << endl;
  }
}


