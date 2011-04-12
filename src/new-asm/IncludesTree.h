/*
 * IncludesTree.h
 *
 *  Created on: 11/apr/2011
 *      Author: ben
 */

#ifndef INCLUDESTREE_H_
#define INCLUDESTREE_H_

#include "asm-classes.h"

#include <string>
#include <vector>

using namespace std;

class IncludesNode {
  IncludesNode * parent;

  const string filepath;
  const string filename;
  const YYLTYPE inclusionPosition;

  vector<IncludesNode *> included;

  void clean() {
    for(size_t child = 0; child != included.size(); child++)
    {
      delete included[child];
    }
  }
public:
  IncludesNode(IncludesNode * _parent, const char * _path, const char * _name,
                const YYLTYPE & _pos)
    : parent(_parent), filepath(_path), filename(_name), inclusionPosition(_pos)
  { }
  ~IncludesNode() { clean(); }

  const char * getName() const throw() { return filename.c_str(); }
  const char * getPath() const throw() { return filepath.c_str(); }

  const YYLTYPE &getInclusionPosition() const throw() {
    return inclusionPosition;
  }

  bool hasName(const char * name) const throw() {
    return !filename.compare(name);
  }
  bool hasPath(const char * path) const throw() {
    return !filepath.compare(path);
  }

  void attach(IncludesNode * child) { included.push_back(child); }
  IncludesNode * getParent() const { return parent; }
};

class IncludesTree {
  IncludesNode * head;
  IncludesNode * current;

  bool isInParents(const char * _path, const char * _name) const;
public:
  IncludesTree(const char * const _path, const char * const _name);
  ~IncludesTree() { delete head; }

  void enterIncludeOfCurrent(const char * const _path, const char * const _name,
      const YYLTYPE & _pos);
  void exitInclude();

  const char * getCurrentName() const throw() { return current->getName(); }
  const char * getCurrentPath() const throw() { return current->getPath(); }

  const YYLTYPE &getCurrentInclusionPosition() const throw() {
    return current->getInclusionPosition();
  }

  void printStderrCurrent() const;
  void printStderrStackIncludes() const;
};

#endif /* INCLUDESTREE_H_ */
