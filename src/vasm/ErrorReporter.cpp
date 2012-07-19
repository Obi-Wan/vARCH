/*
 * ErrorReporter.cpp
 *
 *  Created on: 20/lug/2012
 *      Author: ben
 */

#include "ErrorReporter.h"

#include <iostream>
#include <sstream>


void
ErrorReporter::addErrorMsg(const string && msg)
{
  this->listOfErrMsgs.push_back(move(msg));
}

void
ErrorReporter::addErrorMsg(const YYLTYPE & pos, const BasicException & e)
{
  stringstream stream;
  stream << "ERROR:" << pos.fileNode->printString() << " at Line ";
  stream.width(4);
  stream << pos.first_line;
  stream.width(0);
  stream << endl << pos.fileNode->printStringStackIncludes() << endl;
  stream << "--> " << e.what() << endl;

  this->addErrorMsg(stream.str());
}

void
ErrorReporter::throwError(BasicException && e)
{
  stringstream stream;
  stream << endl;
  for(string & str : listOfErrMsgs) { stream << str << endl; }
  e.appendMessage(stream.str());

  throw e;
}

void
ErrorReporter::printError(const YYLTYPE & pos, const BasicException & e)
{
  fprintf(stderr, "ERROR:%s at Line %4d\n%s\n--> %s\n",
          pos.fileNode->printString().c_str(), pos.first_line,
          pos.fileNode->printStringStackIncludes().c_str(),
          e.what());
}

