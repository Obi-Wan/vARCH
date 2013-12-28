/*
 * ObjHandler.cpp
 *
 *  Created on: 13/feb/2013
 *      Author: ben
 */

#include "ObjHandler.h"

#include <elfio/elfio.hpp>

using namespace ELFIO;

#include <map>
#include <string>

template<typename SymbolNameType = string, typename SymbolType = Elf_Word>
class SymbolsTempLookup {
  typedef std::map<SymbolType, SymbolNameType> SymToStr;
  typedef std::map<SymbolNameType, SymbolType> StrToSym;
  SymToStr symToStr;
  StrToSym strToSym;
public:
  SymbolsTempLookup() = default;

  void addSymbol(const SymbolNameType & str, const SymbolType & sym);

  const SymbolType & getSymbol(const SymbolNameType && str);
  const SymbolType & getSymbol(const SymbolNameType & str);
  const SymbolNameType & getString(const SymbolType & sym);

  bool isSymbol(const SymbolNameType & str);
  bool isSymbol(const SymbolType & sym);
};

template<typename SymbolNameType, typename SymbolType>
INLINE void
SymbolsTempLookup<SymbolNameType, SymbolType>::addSymbol(
    const SymbolNameType & str, const SymbolType & sym)
{
  if (strToSym.find(str) == strToSym.end()) {
    symToStr.insert(typename SymToStr::value_type(sym, str));
    strToSym.insert(typename StrToSym::value_type(str, sym));
  } else {
    throw WrongArgumentException("String '" + str + "' is already a symbol");
  }
}

template<typename SymbolNameType, typename SymbolType>
INLINE const SymbolType &
SymbolsTempLookup<SymbolNameType, SymbolType>::getSymbol(
    const SymbolNameType & str)
{
  const typename StrToSym::iterator iter = strToSym.find(str);
  if (iter != strToSym.end()) {
    return iter->second;
  } else {
    throw WrongArgumentException("String '" + str + "' is not a symbol");
  }
}

template<typename SymbolNameType, typename SymbolType>
INLINE const SymbolType &
SymbolsTempLookup<SymbolNameType, SymbolType>::getSymbol(
    const SymbolNameType && str)
{
  const typename StrToSym::iterator iter = strToSym.find(move(str));
  if (iter != strToSym.end()) {
    return iter->second;
  } else {
    throw WrongArgumentException("String '" + str + "' is not a symbol");
  }
}

template<typename SymbolNameType, typename SymbolType>
INLINE const SymbolNameType &
SymbolsTempLookup<SymbolNameType, SymbolType>::getString(
    const SymbolType & sym)
{
  const typename SymToStr::iterator iter = symToStr.find(sym);
  if (iter != symToStr.end()) {
    return iter->second;
  } else {
    throw WrongArgumentException("Symbol: " + std::to_string(sym) + " is not known");
  }
}

template<typename SymbolNameType, typename SymbolType>
INLINE bool
SymbolsTempLookup<SymbolNameType, SymbolType>::isSymbol(
    const SymbolNameType & str)
{
  return (strToSym.find(str) != strToSym.end());
}

template<typename SymbolNameType, typename SymbolType>
INLINE bool
SymbolsTempLookup<SymbolNameType, SymbolType>::isSymbol(
    const SymbolType & sym)
{
  return (symToStr.find(sym) != symToStr.end());
}

////////////////////////////////////////////////////////////////////////////////
/// READ OBJ

void
ObjLoader::readObj(asm_program & prog)
{

}

////////////////////////////////////////////////////////////////////////////////
/// WRITE OBJ

void
ObjWriter::writeObj(const asm_program & prog)
{
  elfio elf_writer;
  SymbolsTempLookup<> lookup_string;
  SymbolsTempLookup<> lookup_symbol;

  elf_writer.create( ELFCLASS32, ELFDATA2LSB );

  elf_writer.set_os_abi( ELFOSABI_NONE );
  elf_writer.set_type( ET_REL );
  elf_writer.set_machine( EM_NONE );

  // Create Instructions Section
  section * sec_text = elf_writer.sections.add(".text");
  sec_text->set_type( SHT_PROGBITS );
  sec_text->set_flags( SHF_ALLOC | SHF_EXECINSTR );
  sec_text->set_addr_align( 0x04 );

  // Create Data Section
  section * sec_data = elf_writer.sections.add(".data");
  sec_data->set_type( SHT_PROGBITS );
  sec_data->set_flags( SHF_ALLOC | SHF_WRITE );
  sec_data->set_addr_align( 0x04 );

  // Create Read-Only Data Section
  section * sec_rodata = elf_writer.sections.add(".rodata");
  sec_rodata->set_type( SHT_PROGBITS );
  sec_rodata->set_flags( SHF_ALLOC );
  sec_rodata->set_addr_align( 0x04 );

  // Create Strings Table
  section * sec_strings = elf_writer.sections.add(".strtab");
  sec_strings->set_type( SHT_STRTAB );
  sec_strings->set_flags( SHF_ALLOC );

  string_section_accessor accessor_str(sec_strings);

  // Create Table of Symbols
  section * sec_symbols = elf_writer.sections.add(".symtab");
  sec_symbols->set_type( SHT_SYMTAB );
  sec_symbols->set_flags( SHF_ALLOC );
  sec_symbols->set_info( 2 );
  sec_symbols->set_addr_align( 0x4 );
  sec_symbols->set_entry_size( elf_writer.get_default_entry_size( SHT_SYMTAB ) );
  sec_symbols->set_link( sec_strings->get_index() );

  symbol_section_accessor accessor_sym(elf_writer, sec_symbols);

  // Create References Section
  section * sec_rel_text = elf_writer.sections.add(".rel.text");
  sec_rel_text->set_type( SHT_REL );
  sec_rel_text->set_info( sec_text->get_index() );
  sec_rel_text->set_addr_align( 0x4 );
  sec_rel_text->set_entry_size( elf_writer.get_default_entry_size( SHT_REL ) );
  sec_rel_text->set_link( sec_symbols->get_index() );

  relocation_section_accessor accessor_rel(elf_writer, sec_rel_text);

  // Write strings (symbol names) in Stings Table
  for (LabelsMap::value_type label_pair : prog.globalSymbols.getLabels())
  {
    const asm_label_statement * label_stmt = label_pair.second;

    // Adding String
    Elf_Word sym_name = accessor_str.add_string(label_stmt->label);
    lookup_string.addSymbol(label_stmt->label, sym_name);
  }

  // Write symbols in Table
  for (LabelsMap::value_type label_pair : prog.globalSymbols.getLabels())
  {
    const asm_label_statement * label_stmt = label_pair.second;
    Elf_Word sym_name = lookup_string.getSymbol(label_stmt->label);

    Elf_Half label_section_index = label_stmt->is_func
        ? sec_text->get_index()
        : (label_stmt->is_constant ? sec_rodata->get_index() : sec_data->get_index());

    unsigned char label_visib = label_stmt->is_global ? STB_GLOBAL : STB_LOCAL;
    unsigned char label_type = label_stmt->is_func ? STT_FUNC : STT_OBJECT;

    const Elf_Xword label_size = label_stmt->size * label_stmt->num;

    DebugPrintf(("Added string: 0x%02x '%s'\n", sym_name, label_stmt->label.c_str()));

    // Adding Symbol
    Elf_Word sym = accessor_sym.add_symbol(sym_name, label_stmt->offset,
        label_size, label_visib, label_type, 0, label_section_index);
    lookup_symbol.addSymbol(label_stmt->label, sym);

    DebugPrintf(("Added symbol: 0x%02x '%s'\n", sym, label_stmt->label.c_str()));
  }

  // Write function blocks
  for (asm_function * func : prog.functions)
  {
    Bloat data;
    data.resize(func->getSize());
    Bloat::iterator pos = data.begin();

    for(asm_statement * stmt : func->stmts) { stmt->emitCode(pos); }

    sec_text->append_data((const char *)&data[0], data.size());

    for (ArgLabelRecord * ref : func->refs)
    {
      if (lookup_string.isSymbol(ref->arg->label))
      {
        Elf_Word sym = lookup_symbol.getSymbol(ref->arg->label);
        Elf64_Addr offset = ref->arg->relOffset + ref->parent->offset;

        DebugPrintf(("Reference to symbol: sym 0x%02x, '%s', offset 0x%04x\n",
            sym, ref->arg->label.c_str(), offset));

        accessor_rel.add_entry(offset, sym, (unsigned char)R_386_NONE);
      }
    }
  }

  // Write Shared (static) Variables
  for (asm_data_statement * stmt : prog.shared_vars)
  {
    Bloat data;
    data.resize(stmt->getSize());
    Bloat::iterator pos = data.begin();
    stmt->emitCode(pos);
    sec_data->append_data((const char *)&data[0], data.size());
  }

  // Write Constants
  for (asm_data_statement * stmt : prog.constants)
  {
    Bloat data;
    data.resize(stmt->getSize());
    Bloat::iterator pos = data.begin();
    stmt->emitCode(pos);
    sec_rodata->append_data((const char *)&data[0], data.size());
  }

  elf_writer.save(this->filename);
}

