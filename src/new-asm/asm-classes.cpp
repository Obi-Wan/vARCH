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
  throw(WrongArgumentException)
{
  DebugPrintf(("TableOfSymbols: adding label at position %03d: %s\n",
         lab->offset, lab->label.c_str()));
  const LabelsMap::iterator previousDeclaration =
      defLabels.find(lab->label);
  if (previousDeclaration == defLabels.end()) {
    defLabels.insert( LabelsMap::value_type( lab->label, lab ) );
  } else {
    stringstream stream;
    stream  << "Multiple definitions of '" << lab->label << "' at positions:\n"
            << " - Line: " << previousDeclaration->second->position.first_line
            << " The first definition.\n - Line: "
            << lab->position.first_line << " The new definition.\n";
    throw WrongArgumentException(stream.str());
  }
}
