/*
 * IncludesTree.cpp
 *
 *  Created on: 11/apr/2011
 *      Author: ben
 */

#include "IncludesTree.h"

#include "exceptions.h"

#include <cstdio>

IncludesTree::IncludesTree(const char * const _path, const char * const _name)
{
  const YYLTYPE _pos = { 1, 1, 1, 1, "", "" };
  head = new IncludesNode(NULL, _path, _name, _pos);
  current = head;
}

bool
IncludesTree::isInParents(const char * _path, const char * _name) const
{
  for(IncludesNode * parent = current; parent; parent = parent->getParent())
  {
    if (parent->hasPath(_path) && parent->hasName(_name)) { return true; }
  }
  return false;
}

void
IncludesTree::enterIncludeOfCurrent(const char * const _path,
    const char * const _name, const YYLTYPE & _pos)
{
  if (!isInParents(_path, _name)) {
    IncludesNode * leaf = new IncludesNode(current, _path, _name, _pos);
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
IncludesTree::printStderrCurrent() const
{
  fprintf( stderr, "  in file '%s'\n", current->getName() );
}

void
IncludesTree::printStderrStackIncludes() const
{
  YYLTYPE includedPos = current->getInclusionPosition();
  for(IncludesNode * parent = current->getParent(); parent;
      parent = parent->getParent())
  {
    fprintf(stderr, "  Included by: '%s/%s' (at line: %4d)\n",
            parent->getPath(), parent->getName(), includedPos.first_line );
    includedPos = parent->getInclusionPosition();
  }
}

