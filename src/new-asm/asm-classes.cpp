/*
 * asm-classes.cpp
 *
 *  Created on: 20/mar/2011
 *      Author: ben
 */

#include "asm-classes.h"

#include <sstream>
using namespace std;

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
            << " - File: '" << prevPos.filepath << "/" << prevPos.filename
              << "' Line: " << prevPos.first_line << " The first definition.\n"
            << " - File: '" << pos.filepath << "/" << pos.filename
              << "' Line: " << pos.first_line << " The new definition.\n";
    throw WrongArgumentException(stream.str());
  }
}
