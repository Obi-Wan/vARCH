/* 
 * File:   AsmParser.cpp
 * Author: ben
 * 
 * Created on 27 agosto 2009, 14.26
 */

#include "AsmParser.h"
#include "../include/macros.h"
#include "FunctionRecord.h"

AsmParser::AsmParser(const string& _fname) : fname(_fname) {
  TextLoader loader(fname.data());
  preParser(loader.getTextFileContent());
}

//AsmParser::AsmParser(const AsmParser& orig) { }

//AsmParser::~AsmParser() { }

inline void
AsmParser::addAllChunks(istringstream& inputLine, LineOfCode& outputLine) {
  string word;
  for(; (inputLine >> word) && !word.empty(); ) {
    outputLine.chunks.push_back(word);
  }
}

inline void
AsmParser::preParser(const string& fileContent) {
  istringstream fileStr(fileContent);
  // Scans over the lines
  DebugPrintf(("Starting preParsing\n"));
  for(unsigned int lineNum = 0; !fileStr.eof(); lineNum++) {
    LineOfCode lineOfCode;
    string word, line;

    // Extract line and skip if is comment or blank
    getline(fileStr, line);
    if (line.empty() || line[0] == ';') continue;

    // Extract first word and skip if it's comment or blank
    istringstream lineStream(line);
    lineStream >> word;
    if (word.empty() || word[0] == ';') continue;

    lineOfCode.chunks.push_back(word);
    // now if it's a constant we save it's content all together
    if (word[0] == '.') {
      // first case: not a label
      if (word[word.size()-1] != ':') {
        const Marker::MarkersType marker = Marker::getMarkerType(word);
        if (marker < Marker::GLOBAL) {
          lineStream.ignore(256,'$');
          string constString;
          getline(lineStream, constString);
          lineOfCode.chunks.push_back(constString);
        } else {
          addAllChunks(lineStream, lineOfCode);
        }

        switch (marker) {
          case Marker::STRING:
            lineOfCode.bytes = lineOfCode.chunks.at(1).size();
            break;
          case Marker::LONG:
            lineOfCode.bytes = 2;
            break;
          case Marker::INT:
          case Marker::CHAR:
            lineOfCode.bytes = 1;
            break;
          default:
            lineOfCode.bytes = 0;
            break;
        }
      } else {
        // this is a label, so no bytes are occupied
        lineOfCode.bytes = 0;
      }
    // otherwise let's add it direcly without processing
    } else {
      addAllChunks(lineStream, lineOfCode);
      lineOfCode.bytes = lineOfCode.chunks.size();
    }
    DebugPrintf(("adding the line %d to the organizer\n", lineNum));
    organizer.addLine(lineOfCode);
  }
}

inline void
AsmParser::synthaxParser() {
  //FIXME it's just useless now
}

void
AsmParser::parse() throw(WrongArgumentException, WrongIstructionException) {

  organizer.assignGlobalSymbols();
  organizer.parseFunctions();

  printf("Parsing Finished\n");
}

void
AsmParser::assemble() {
  organizer.assemble(code);
}
