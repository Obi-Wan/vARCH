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

struct PreprocessorDefine {
  std::list<std::string> parameters;
  std::string content;

  PreprocessorDefine() = default;
  PreprocessorDefine(PreprocessorDefine &&) = default;
  PreprocessorDefine(const PreprocessorDefine &) = default;
  PreprocessorDefine(const std::list<std::string> &, const std::string &);
  PreprocessorDefine(const std::list<std::string> &&, const std::string &&);
};

typedef std::map<std::string, PreprocessorDefine> DefineType;

class AsmPreprocessor {
  DefineType defines;
public:
  AsmPreprocessor() = default;

  void addDefine(const std::string && _name, const std::string && _value);
  void addDefine(const std::string && _name, const std::string && _value, std::list<std::string> * params);
  void delDefine(const std::string && _name) {
    defines.erase(_name);
  }

  bool isDefined(const std::string & _name) const {
    const DefineType::const_iterator result = defines.find(_name);
    return (result != defines.end());
  }

  size_t getDefineParameter(const std::string & _name, const std::string & id) const {
    const DefineType::const_iterator result = defines.find(_name);
    const std::list<std::string> & params = result->second.parameters;

    size_t count = 0;
    for (const std::string & param : params) {
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
