/* 
 * File:   CodeOrganizer.cpp
 * Author: ben
 * 
 * Created on 12 dicembre 2009, 15.40
 */

#include "CodeOrganizer.h"

//CodeOrganizer::CodeOrganizer() { }
//
//CodeOrganizer::CodeOrganizer(const CodeOrganizer& orig) { }
//
//CodeOrganizer::~CodeOrganizer() { }


void
CodeOrganizer::storeLabel(const string& word, const int& bytePos, const int& lineNum)
            throw(DuplicateLabelException)
{
  string label = word.substr(1,word.size()-2);
  
  if (labels.find(label) != labels.end()) {
    stringstream streamError(string(""));
    streamError << "Line " << lineNum << ": Label already defined at line: "
                << labels.find(label)->second.lineNumber;
    throw DuplicateLabelException(streamError.str());
  }
  labels.insert(Labels::value_type(label, Label(word, lineNum, bytePos)));
}

void
CodeOrganizer::addLine(const CodeLines::value_type &line) throw(DuplicateLabelException) {
  if (global) {
    globals.push_back(line);
  } else {
    const string& word = line.chunks.at(0);
    if ((word[0] == '.') && (word[word.size()-1] != ':')) {
      DebugPrintf(("CodeOrg: adding a marker %s\n",word.c_str()));
      const Marker::MarkersType marker = Marker::getMarkerType(word);
      switch (marker) {
        case Marker::FUNCTION: {
          const string& funcName = line.chunks.at(1);
          DebugPrintf(("Function name: %s\n", funcName.c_str()));
          if (funcName.compare("main") != 0) {
            if (labels.find(funcName) != labels.end()) {
              throw DuplicateLabelException("Function/Label " + funcName +
                      " already defined");
            }
            tempFunc = new FunctionRecord(funcName, labels);
          } else if (main != NULL) {
            stringstream streamError(string(""));
            streamError << "Line " << line.lineNumber << "No free code/main "
                        << "allowed here: a main was already defined";
            throw DuplicateLabelException(streamError.str());
          } else {
            tempFunc = new FunctionRecord("main", labels);
          }
          break;
        }
        case Marker::GLOBAL: {
          DebugPrintf(("Setting global\n"));
          global = true;
          if (tempFunc) {
            closeCurrentFunction(line.lineNumber);
          }
          break;
        }
        case Marker::END: {
          closeCurrentFunction(line.lineNumber);
          break;
        }
        default:
          tempFunc->addLine(line);
          break;
      }
    } else {
      DebugPrintf(("CodeOrg: code istr/label %s\n",word.c_str()));
      if (tempFunc != NULL) {
        tempFunc->addLine(line);
      } else if (main == NULL) {
        tempFunc = new FunctionRecord("main", labels);
        tempFunc->addLine(line);
      }
    }
  }
}

inline void
CodeOrganizer::closeCurrentFunction(const int& lineNum) {
  if (!tempFunc) {
    stringstream streamError(string(""));
    streamError << "Line: " << lineNum << "end of function when"
                << " not in function";
    throw WrongIstructionException(streamError.str());
  }
  DebugPrintf(("Reached End of %s\n", tempFunc->getName().c_str()));
  if (tempFunc->getName().compare("main") != 0) {
    functions.push_back(tempFunc);
  } else {
    main = tempFunc;
  }
  tempFunc = NULL;
}

void
CodeOrganizer::assignGlobalSymbols() {
  DebugPrintf(("Starting assigning global symbols\n"));
  unsigned int bytePos = 0;
  if (main != NULL) {
    storeLabel(".main:", bytePos, 0);
    bytePos += main->getBytes();
    DebugPrintf(("main present. bytePos at: %d\n", bytePos));
  }
  for (unsigned int i = 0; i < functions.size(); i++) {
    storeLabel("." +functions[i]->getName()+":", bytePos, -1); // TODO FIXME
    bytePos += functions[i]->getBytes();
    DebugPrintf(("counted function: %s. bytePos at: %d\n",
                  functions[i]->getName().c_str(), bytePos));
  }
  // and now constants! :)
  for (unsigned int i = 0; i < globals.size(); i++) {
    const string& word = globals[i].chunks.at(0);
    if (word[word.size()-1] == ':') {
      storeLabel(word, bytePos, globals[i].lineNumber);
    } else {
      switch (Marker::getMarkerType(word)) {
        case Marker::INT: {
          istringstream argStr( globals[i].chunks.at(1) );
          int value;
          argStr >> value;
          consts.push_back(value);
          bytePos++;
          break;
        }
        case Marker::LONG:
          throw WrongArgumentException("Long type not yet supported");
        case Marker::CHAR:
        case Marker::STRING: {
          const string& temp = globals[i].chunks.at(1);
          for (unsigned int i = 0; i < temp.size(); i++) {
            consts.push_back(temp[i] & 0xff );
            bytePos++;
          }
          break;
        }
        default:
          throw WrongArgumentException("Only constants after a .global mark");
      }
    }
  }
  DebugPrintf(("finished assigning global labels positions\n"));
  DebugPrintfLabels(labels, "Labels");
}

void
CodeOrganizer::parseFunctions() {
  if (main) {
    main->preProcess();
  }
  for (unsigned int i = 0; i < functions.size(); i++) {
    functions[i]->preProcess();
  }
}

void
CodeOrganizer::assemble(Bloat& code) {
  if (main) {
    main->assemble();
    code.insert(code.end(), main->getCode().begin(), main->getCode().end());
  }
  for(unsigned int i = 0; i < functions.size(); i++) {
    functions[i]->assemble();
    code.insert(code.end(), functions[i]->getCode().begin(), functions[i]->getCode().end());
  }
  for (unsigned int i = 0; i < consts.size(); i++) {
    code.push_back(consts[i]);
  }
}

