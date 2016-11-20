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
ErrorReporter::addErrorMsg(const std::string && msg)
{
  this->listOfErrMsgs.push_back(move(msg));
}

void
ErrorReporter::addErrorMsg(const YYLTYPE & pos, const std::string && err)
{
  std::stringstream stream;
  stream << "ERROR:" << pos.fileNode->printString() << " at Line ";
  stream.width(4);
  stream << pos.first_line;
  stream.width(0);
  if (pos.fileNode)
  {
    stream << std::endl << pos.fileNode->printStringStackIncludes() << std::endl;
  }
  stream << "--> " << err << std::endl;

  this->addErrorMsg(stream.str());
}

void
ErrorReporter::addErrorMsg(const YYLTYPE & pos, const BasicException & e)
{
  std::stringstream stream;
  stream << "ERROR:" << pos.fileNode->printString() << " at Line ";
  stream.width(4);
  stream << pos.first_line;
  stream.width(0);
  stream << std::endl << pos.fileNode->printStringStackIncludes() << std::endl;
  stream << "--> " << e.what() << std::endl;

  this->addErrorMsg(stream.str());
}

void
ErrorReporter::throwError(BasicException && e)
{
  std::stringstream stream;
  stream << std::endl;
  for(std::string & str : listOfErrMsgs) { stream << str << std::endl; }
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

