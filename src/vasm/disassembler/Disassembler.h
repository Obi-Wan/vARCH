/*
 * Disassembler.h
 *
 *  Created on: 02/ago/2011
 *      Author: ben
 */

#ifndef DISASSEMBLER_H_
#define DISASSEMBLER_H_

#include "../asm-program.h"

#include <map>
#include <string>

template<typename SymbolNameType = std::string, typename SymbolType = size_t>
class OffsetTempLookup {
protected:
  typedef std::map<SymbolType, SymbolNameType> SymToStr;
  SymToStr symToStr;
public:
  OffsetTempLookup() = default;

  void addOffset(const SymbolNameType & str, const SymbolType & sym);

  const SymbolNameType & getString(const SymbolType & sym) const;

  bool isOffset(const SymbolType & sym) const;
};

template<typename SymbolNameType = std::string, typename SymbolType = size_t>
class SymbolsTempLookup : public OffsetTempLookup<SymbolNameType, SymbolType> {
protected:
  typedef typename OffsetTempLookup<SymbolNameType, SymbolType>::SymToStr SymToStr;
  typedef std::map<SymbolNameType, SymbolType> StrToSym;
  StrToSym strToSym;
public:
  SymbolsTempLookup() = default;

  void addSymbol(const SymbolNameType & str, const SymbolType & sym);

  const SymbolType & getSymbol(const SymbolNameType && str) const;
  const SymbolType & getSymbol(const SymbolNameType & str) const;

  bool isSymbol(const SymbolNameType & str) const;
  bool isSymbol(const SymbolType & sym) const;
};

class Disassembler {
  void printArg(const TypeOfArgument & typeArg,
      const ScaleOfArgument & scaleArg, const ArgumentValue & arg);
  void printLocals(const ListOfDataStmts & locals, const size_t & funcOffset) const;

  ArgumentValue fetchArg(const TypeOfArgument & typeArg,
      const ScaleOfArgument & scaleArg, const int8_t *& codeIt,
      const int8_t * const endIt);
  asm_arg * decodeArgument(const TypeOfArgument & p_arg_type,
      const ScaleOfArgument & p_arg_scale, const ArgumentValue & arg);
  void decodeInstruction(const int8_t *& data, uint32_t & instr,
      std::vector<TypeOfArgument> & type_args, std::vector<ScaleOfArgument> & scale_args);
public:
//  Disassembler();

  void disassembleBytecode(asm_program & prog, const int8_t * data,
      const size_t & size, const SymbolsTempLookup<> & lookup_func,
      const OffsetTempLookup<> & lookup_rel);
  void disassembleAndPrint(const Bloat & bytecode);
  void disassembleProgram(const asm_program & prog);
};


////////////////////////////////////////////////////////////////////////////////
/// Methods
////////////////////////////////////////////////////////////////////////////////

template<typename SymbolNameType, typename SymbolType>
INLINE void
OffsetTempLookup<SymbolNameType, SymbolType>::addOffset(
    const SymbolNameType & str, const SymbolType & sym)
{
  if (symToStr.find(sym) == symToStr.end()) {
    symToStr.insert(typename SymToStr::value_type(sym, str));
  } else {
    throw WrongArgumentException("Offset '" + std::to_string(sym) + "' is already a symbol");
  }
}

template<typename SymbolNameType, typename SymbolType>
INLINE void
SymbolsTempLookup<SymbolNameType, SymbolType>::addSymbol(
    const SymbolNameType & str, const SymbolType & sym)
{
  if (strToSym.find(str) == strToSym.end()) {
    this->addOffset(str, sym);
    strToSym.insert(typename StrToSym::value_type(str, sym));
  } else {
    throw WrongArgumentException("String '" + str + "' is already a symbol");
  }
}

template<typename SymbolNameType, typename SymbolType>
INLINE const SymbolType &
SymbolsTempLookup<SymbolNameType, SymbolType>::getSymbol(
    const SymbolNameType & str) const
{
  const typename StrToSym::const_iterator iter = strToSym.find(str);
  if (iter != strToSym.end()) {
    return iter->second;
  } else {
    throw WrongArgumentException("String '" + str + "' is not a symbol");
  }
}

template<typename SymbolNameType, typename SymbolType>
INLINE const SymbolType &
SymbolsTempLookup<SymbolNameType, SymbolType>::getSymbol(
    const SymbolNameType && str) const
{
  const typename StrToSym::const_iterator iter = strToSym.find(move(str));
  if (iter != strToSym.end()) {
    return iter->second;
  } else {
    throw WrongArgumentException("String '" + str + "' is not a symbol");
  }
}

template<typename SymbolNameType, typename SymbolType>
INLINE const SymbolNameType &
OffsetTempLookup<SymbolNameType, SymbolType>::getString(
    const SymbolType & sym) const
{
  const typename SymToStr::const_iterator iter = symToStr.find(sym);
  if (iter != symToStr.end()) {
    return iter->second;
  } else {
    throw WrongArgumentException("Symbol: " + std::to_string(sym) + " is not known");
  }
}

template<typename SymbolNameType, typename SymbolType>
INLINE bool
SymbolsTempLookup<SymbolNameType, SymbolType>::isSymbol(
    const SymbolNameType & str) const
{
  return (strToSym.find(str) != strToSym.end());
}

template<typename SymbolNameType, typename SymbolType>
INLINE bool
OffsetTempLookup<SymbolNameType, SymbolType>::isOffset(
    const SymbolType & sym) const
{
  return (symToStr.find(sym) != symToStr.end());
}

template<typename SymbolNameType, typename SymbolType>
INLINE bool
SymbolsTempLookup<SymbolNameType, SymbolType>::isSymbol(
    const SymbolType & sym) const
{
  return this->isOffset(sym);
}

#endif /* DISASSEMBLER_H_ */
