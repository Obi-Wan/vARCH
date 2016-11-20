/*
 * IncludesTree.cpp
 *
 *  Created on: 11/apr/2011
 *      Author: ben
 */

#include "IncludesTree.h"

#include "exceptions.h"

#include <cstdio>
#include <sstream>

IncludesTree::IncludesTree(const char * const _path, const char * const _name)
{
  const YYLTYPE _pos = { 1, 1, 1, 1, NULL };
  head = new IncludesNode(_path, _name, _pos);
  current = head;
}

bool
IncludesTree::isInParents(const char * _path, const char * _name) const
{
  for(const IncludesNode * prnt = current; prnt; prnt = prnt->getParent())
  {
    if (prnt->hasPath(_path) && prnt->hasName(_name)) { return true; }
  }
  return false;
}

void
IncludesTree::enterIncludeOfCurrent(const char * const _path,
    const char * const _name, const YYLTYPE & _pos)
{
  if (!isInParents(_path, _name)) {
    IncludesNode * leaf = new IncludesNode(_path, _name, _pos);
    current->attach(leaf);
    current = leaf;
  } else {
    throw WrongFileException("Circular inclusion");
  }
}

void
IncludesTree::exitInclude()
{
  IncludesNode * parent = current->getParent();
  if (parent) {
    current = parent;
  } else {
    // boooooooooooooooooooooo
  }
}

void
IncludesNode::printStderr() const
{
  fprintf( stderr, "  in file '%s/%s'\n", getPath(), getName() );
}

std::string
IncludesNode::printString() const
{
  std::stringstream stream;
  stream << "  in file: '" << getPath() << "/" << getName() << "'";
  return stream.str();
}

void
IncludesNode::printStderrStackIncludes() const
{
  YYLTYPE includedPos = this->getInclusionPosition();
  for(IncludesNode * parent = this->getParent(); parent;
      parent = parent->getParent())
  {
    fprintf(stderr, "  Included by: '%s/%s' (at line: %4d)\n",
            parent->getPath(), parent->getName(), includedPos.first_line );
    includedPos = parent->getInclusionPosition();
  }
}

std::string
IncludesNode::printStringStackIncludes() const
{
  std::stringstream stream;
  YYLTYPE includedPos = this->getInclusionPosition();
  for(IncludesNode * prnt = this->getParent(); prnt;
      prnt = prnt->getParent())
  {
    stream  << "  Included by: '" << prnt->getPath() << "/" << prnt->getName()
              << "' (at line: " << includedPos.first_line << ")";
    includedPos = prnt->getInclusionPosition();
  }
  return stream.str();
}

