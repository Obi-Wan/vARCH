/*
 * Labels.cpp
 *
 *  Created on: 27/lug/2011
 *      Author: ben
 */

#include "Labels.h"
#include "../IncludesTree.h"
#include "exceptions.h"

#include <sstream>
using namespace std;

void
TableOfSymbols::addLabel(asm_label_statement * lab)
{
  DebugPrintf(("TableOfSymbols: adding label at position %03lu: %s\n",
         (uint64_t)lab->offset, lab->label.c_str()));
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

string
TableOfSymbols::emitDebugSymbols() const
{
  stringstream output;
  for(LabelsMap::const_iterator labelIt = defLabels.begin();
      labelIt != defLabels.end(); labelIt++)
  {
    output << "  \"" << labelIt->first << "\""
            << "  " << labelIt->second->offset << endl;
  }
  return output.str();
}

string
TableOfSymbols::emitXMLDebugSymbols() const
{
  stringstream output;
  for(LabelsMap::const_iterator labelIt = defLabels.begin();
      labelIt != defLabels.end(); labelIt++)
  {
    output << "  <symbol>" << endl
            << "    <id>" << labelIt->first << "</id>" << endl
            << "    <offset>" << labelIt->second->offset << "</offset>" << endl
            << "  </symbol>" << endl;
  }
  return output.str();
}

