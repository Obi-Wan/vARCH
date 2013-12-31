/*
 * ObjHandler.cpp
 *
 *  Created on: 13/feb/2013
 *      Author: ben
 */

#include "ObjHandler.h"

#include "disassembler/Disassembler.h"

#include <elfio/elfio.hpp>

using namespace ELFIO;

////////////////////////////////////////////////////////////////////////////////
/// READ OBJ

void
ObjLoader::readObj(asm_program & prog)
{
  elfio elf_reader;
  SymbolsTempLookup<> lookup_position_func;
  SymbolsTempLookup<> lookup_position_shared;
  SymbolsTempLookup<> lookup_position_const;
  OffsetTempLookup<> lookup_rel;

  // Load ELF data
  if ( !elf_reader.load( this->filename ) )
  {
    WrongFileException("Can't find or process ELF file: " + this->filename );
  }

  if (elf_reader.get_class() != ELFCLASS32)
  {
    WrongFileException("File: '" + this->filename + "' is not 32bits");
  }
  if (elf_reader.get_encoding() != ELFDATA2LSB)
  {
    WrongFileException("File: '" + this->filename + "' is not Little-endian");
  }

  section * sec_text = elf_reader.sections[".text"];
  if (!sec_text)
  {
    WrongFileException("File: '" + this->filename + "' does not contain a '.text' section");
  }
  section * sec_data = elf_reader.sections[".data"];
  if (!sec_data)
  {
    WrongFileException("File: '" + this->filename + "' does not contain a '.data' section");
  }
  section * sec_rodata = elf_reader.sections[".rodata"];
  if (!sec_rodata)
  {
    WrongFileException("File: '" + this->filename + "' does not contain a '.rodata' section");
  }
  section * sec_strings = elf_reader.sections[".strtab"];
  if (!sec_strings)
  {
    WrongFileException("File: '" + this->filename + "' does not contain a '.strtab' section");
  }
  section * sec_symbols = elf_reader.sections[".symtab"];
  if (!sec_symbols)
  {
    WrongFileException("File: '" + this->filename + "' does not contain a '.symtab' section");
  }
  section * sec_rel_text = elf_reader.sections[".rel.text"];
  if (!sec_rel_text)
  {
    WrongFileException("File: '" + this->filename + "' does not contain a '.rel.text' section");
  }

  {
    symbol_section_accessor accessor_sym(elf_reader, sec_symbols);
    // Let's load symbols
    DebugPrintf(("\nSymbol Section size: %lu\n", accessor_sym.get_symbols_num()-1));
    for (Elf_Half indx = 1; indx < accessor_sym.get_symbols_num(); indx++)
    {
      std::string name;
      Elf64_Addr value;
      Elf_Xword size;
      unsigned char bind;
      unsigned char type;
      Elf_Half section_index;
      unsigned char other;

      accessor_sym.get_symbol( indx, name, value, size, bind, type,
          section_index, other );

      if (type == STT_FUNC)
      {
        DebugPrintf(("Found func symbol: %s\n", name.c_str()));
        lookup_position_func.addSymbol(name, value);
      }
      else
      {
        if (section_index == sec_rodata->get_index())
        {
          DebugPrintf(("Found const symbol: %s\n", name.c_str()));
          lookup_position_const.addSymbol(name, value);
        }
        else
        {
          DebugPrintf(("Found shared symbol: %s\n", name.c_str()));
          lookup_position_shared.addSymbol(name, value);
        }
      }
      DebugPrintf(("Added symbol: 0x%02x '%s', offset 0x%02lx\n", indx,
          name.c_str(), value));

      asm_label_statement * lab =
          new asm_label_statement(YYLTYPE(), name, size, 1, bind == STB_GLOBAL,
              type == STT_FUNC);
      lab->offset = value;
      prog.globalSymbols.addLabel(lab);
    }
  }

  // Let's load relocation symbols
  {
    relocation_section_accessor accessor_rel(elf_reader, sec_rel_text);
    DebugPrintf(("\nRelocation Section size: %lu\n", accessor_rel.get_entries_num()));
    for (Elf_Xword indx = 0; indx < accessor_rel.get_entries_num(); indx++)
    {
      Elf64_Addr offset;
      Elf64_Addr value;
      string sym_name;

      Elf_Word type;
      Elf_Sxword dummy1;
      Elf_Sxword dummy2;

      accessor_rel.get_entry(indx, offset, value, sym_name, type, dummy1, dummy2);

      lookup_rel.addOffset(sym_name, offset);
      DebugPrintf(("Found relocation ref: offset 0x%04lx (%04lu), '%s'\n",
          offset, offset, sym_name.c_str()));
    }
  }

  // Let's load constants
  {
    const char * rodata_data = sec_rodata->get_data();
    const size_t rodata_size = sec_rodata->get_size();

    DebugPrintf(("\nRODATA size: %lu, p %p\n", rodata_size, rodata_data));

    size_t previous_indx = 0;

    for (size_t indx = 0; indx < rodata_size; indx++)
    {
      if (lookup_position_const.isSymbol(indx))
      {
        if (indx)
        {
          const size_t sym_size = indx - previous_indx;
          const string && sym_content = string(&rodata_data[previous_indx], sym_size);
          asm_string_keyword_statement * stmt = new asm_string_keyword_statement(YYLTYPE(), sym_content);
          prog.constants.push_back(stmt);
          previous_indx = indx;

          DebugPrintf(("Saved const data (size %lu): '%s'\n", sym_size, sym_content.c_str()));
        }

        const string & sym_name = lookup_position_const.getString(indx);
        asm_label_statement * const sym_label = prog.globalSymbols.getStmt(sym_name);
        prog.constants.push_back(sym_label);

        DebugPrintf(("Found const symbol: %s, offset %lu\n", sym_name.c_str(), indx));
      }
    }
    if (rodata_size - previous_indx)
    {
      const size_t sym_size = rodata_size - previous_indx;
      const string && sym_content = string(&rodata_data[previous_indx], sym_size);
      asm_string_keyword_statement * stmt = new asm_string_keyword_statement(YYLTYPE(), sym_content);
      prog.constants.push_back(stmt);
    }
  }

  // Let's load shared variables
  {
    const char * data_data = sec_data->get_data();
    const size_t data_size = sec_data->get_size();

    DebugPrintf(("\nDATA size: %lu, p %p\n", data_size, data_data));

    size_t previous_indx = 0;

    for (size_t indx = 0; indx < data_size; indx++)
    {
      if (lookup_position_shared.isSymbol(indx))
      {
        if (indx)
        {
          const size_t sym_size = indx - previous_indx;
          const string && sym_content = string(&data_data[previous_indx], sym_size);
          asm_string_keyword_statement * stmt = new asm_string_keyword_statement(YYLTYPE(), sym_content);
          prog.shared_vars.push_back(stmt);
          previous_indx = indx;

          DebugPrintf(("Saved shared data (size %lu): '%s'\n", sym_size, sym_content.c_str()));
        }

        const string & sym_name = lookup_position_shared.getString(indx);
        asm_label_statement * const sym_label = prog.globalSymbols.getStmt(sym_name);
        prog.shared_vars.push_back(sym_label);

        DebugPrintf(("Found shared symbol: %s, offset %lu\n", sym_name.c_str(), indx));
      }
    }
    if (data_size - previous_indx)
    {
      const size_t sym_size = data_size - previous_indx;
      const string && sym_content = string(&data_data[previous_indx], sym_size);
      asm_string_keyword_statement * stmt = new asm_string_keyword_statement(YYLTYPE(), sym_content);
      prog.shared_vars.push_back(stmt);
    }
  }

  // Let's load functions
  {
    const char * text_data = sec_text->get_data();
    const size_t text_size = sec_text->get_size();

    DebugPrintf(("\nTEXT size: %lu, p %p\n", text_size, text_data));

    Disassembler().disassembleBytecode(prog, (const int8_t *)text_data, text_size,
        lookup_position_func, lookup_rel);
  }
  DebugPrintf(("\nTerminated loading obj file: %s\n\n", filename.c_str()));
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
      const string & label_name = ref->arg->label;

      asm_label_statement * l_stmt = prog.globalSymbols.getStmt(label_name);
      if (!l_stmt)
      {
        l_stmt = prog.globalSymbols.getStmt(label_name, func->name);
      }

      if (l_stmt)
      {
        Elf_Word sym = lookup_symbol.getSymbol(l_stmt->label);
        Elf64_Addr offset = ref->arg->relOffset + ref->parent->offset + func->functionOffset;

        DebugPrintf(("Reference to symbol: sym 0x%02x, '%s', offset 0x%04lx (%04lu)\n",
            sym, l_stmt->label.c_str(), offset, offset));

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

