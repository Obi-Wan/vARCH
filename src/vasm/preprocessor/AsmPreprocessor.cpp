/*
 * AsmPreprocessor.cpp
 *
 *  Created on: 07/lug/2011
 *      Author: ben
 */

#include "AsmPreprocessor.h"

PreprocessorDefine::PreprocessorDefine(const std::list<std::string> & _pars, const std::string & _cont)
  : parameters(_pars), content(_cont)
{ }

PreprocessorDefine::PreprocessorDefine(const std::list<std::string> && _pars, const std::string && _cont)
  : parameters(_pars), content(_cont)
{ }

void
AsmPreprocessor::addDefine(const std::string && _name, const std::string && _value)
{
  this->addDefine(std::move(_name), std::move(_value), new std::list<std::string>());
}

void
AsmPreprocessor::addDefine(const std::string && _name, const std::string && _value, std::list<std::string> * params)
{
  PreprocessorDefine && def = PreprocessorDefine(move(*(params)), move(_value));
  const DefineType::value_type inTuple(std::move(_name), std::move(def));
  std::pair<DefineType::iterator, bool> result = defines.insert(inTuple);
  if (!result.second) {
    defines.erase(result.first);
    defines.insert(inTuple);
  }
}
