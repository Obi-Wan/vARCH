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
asm_instruction_statement::checkArgs() const {
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
}

void
TableOfSymbols::addLabel(asm_label_statement * lab)
{
  DebugPrintf(("TableOfSymbols: adding label at position %03d: %s\n",
         lab->offset, lab->label.c_str()));
  const LabelsMap::iterator previousDeclaration =
      defLabels.find(lab->label);
  if (previousDeclaration == defLabels.end()) {
    defLabels.insert( LabelsMap::value_type( lab->label, lab ) );
  } else {
    stringstream stream;
    YYLTYPE & prevPos = previousDeclaration->second->position;
    YYLTYPE & pos = lab->position;
    stream  << "Multiple definitions of '" << lab->label << "' at positions:\n"
            << prevPos.fileNode->printString()
              << " Line: " << prevPos.first_line << " The first definition.\n"
              << prevPos.fileNode->printStringStackIncludes() << endl
            << pos.fileNode->printString()
              << " Line: " << pos.first_line << " The new definition.\n"
              << pos.fileNode->printStringStackIncludes() << endl;
    throw WrongArgumentException(stream.str());
  }
}
