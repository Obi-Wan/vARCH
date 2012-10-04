/*
 * AsmPreprocessor.cpp
 *
 *  Created on: 07/lug/2011
 *      Author: ben
 */

#include "AsmPreprocessor.h"

PreprocessorDefine::PreprocessorDefine(const list<string> & _pars, const string & _cont)
  : parameters(_pars), content(_cont)
{ }

PreprocessorDefine::PreprocessorDefine(const list<string> && _pars, const string && _cont)
  : parameters(_pars), content(_cont)
{ }

void
AsmPreprocessor::addDefine(const string && _name, const string && _value)
{
  this->addDefine(move(_name), move(_value), new list<string>());
}

void
AsmPreprocessor::addDefine(const string && _name, const string && _value, list<string> * params)
{
  PreprocessorDefine && def = PreprocessorDefine(move(*(params)), move(_value));
  const DefineType::value_type inTuple(move(_name), move(def));
  pair<DefineType::iterator, bool> result = defines.insert(inTuple);
  if (!result.second) {
    defines.erase(result.first);
    defines.insert(inTuple);
  }
}
