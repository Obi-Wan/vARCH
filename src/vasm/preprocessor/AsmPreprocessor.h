/*
 * AsmPreprocessor.h
 *
 *  Created on: 07/lug/2011
 *      Author: ben
 */

#ifndef ASMPREPROCESSOR_H_
#define ASMPREPROCESSOR_H_

#include <string>
#include <list>
#include <map>

using namespace std;

struct PreprocessorDefine {
  list<string> parameters;
  string content;

  PreprocessorDefine() = default;
  PreprocessorDefine(PreprocessorDefine &&) = default;
  PreprocessorDefine(const PreprocessorDefine &) = default;
  PreprocessorDefine(const list<string> &, const string &);
  PreprocessorDefine(const list<string> &&, const string &&);
};

typedef map<string, PreprocessorDefine> DefineType;

class AsmPreprocessor {
  DefineType defines;
public:
  AsmPreprocessor() = default;

  void addDefine(const string && _name, const string && _value);
  void addDefine(const string && _name, const string && _value, list<string> * params);
  void delDefine(const string && _name) {
    defines.erase(_name);
  }

  bool isDefined(const string & _name) const {
    const DefineType::const_iterator result = defines.find(_name);
    return (result != defines.end());
  }

  size_t getDefineParameter(const string & _name, const string & id) const {
    const DefineType::const_iterator result = defines.find(_name);
    const list<string> & params = result->second.parameters;

    size_t count = 0;
    for (const string & param : params) {
      if (!id.compare(param)) {
        return count+1;
      }
      count++;
    }
    return 0;
  }

  const DefineType & getDefines() const { return defines; }
};

#endif /* ASMPREPROCESSOR_H_ */
