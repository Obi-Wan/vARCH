/*
 * ObjHandler.cpp
 *
 *  Created on: 13/feb/2013
 *      Author: ben
 */

#include "ObjHandler.h"

//#include <map>
//#include <string>
//
//class Symbols {
//  typedef std::map<uint32_t, string> SymToStr;
//  typedef std::map<string, uint32_t> StrToSym;
//  SymToStr symToStr;
//  StrToSym strToSym;
//  uint32_t counter;
//public:
//  Symbols() : counter(0) { }
//  const uint32_t & addSymbol(const string & str);
//  const uint32_t & getSymbol(const string && str);
//  const string & getString(const uint32_t & sym);
//};
//
//class ObjectCodeContainer {
//  Symbols table;
//
//public:
//  void importAsm(const asm_program & prog);
//  void exportAsm(asm_program & prog);
//};
//
//const uint32_t &
//Symbols::addSymbol(const string & str)
//{
//  if (strToSym.find(str) == strToSym.end()) {
//    symToStr.insert(SymToStr::value_type(str, counter));
//    strToSym.insert(StrToSym::value_type(counter, str));
//    return counter++;
//  } else {
//    throw WrongArgumentException("String '" + str + "' is already a symbol");
//  }
//}
//
//const uint32_t &
//Symbols::getSymbol(const string && str)
//{
//  const StrToSym::iterator iter = strToSym.find(move(str));
//  if (iter != strToSym.end()) {
//    return iter->second;
//  } else {
//    throw WrongArgumentException("String '" + str + "' is not a symbol");
//  }
//}
//
//const string &
//Symbols::getString(const uint32_t & sym)
//{
//  const SymToStr::iterator iter = symToStr.find(sym);
//  if (iter != symToStr.end()) {
//    return iter->second;
//  } else {
//    throw WrongArgumentException("Symbol: " + std::to_string(sym) + " is not known");
//  }
//}

void
ObjLoader::readObj(asm_program & prog)
{

}

void
ObjWriter::writeObj(const asm_program & prog)
{
  // Create table of symbols, and write it

  // Write down function blocks

  // Write globals
}

