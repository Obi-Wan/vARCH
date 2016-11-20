/* 
 * File:   FileHandler.h
 * Author: ben
 *
 * Created on 26 agosto 2009, 18.38
 */

#ifndef _FILEHANDLER_H
#define	_FILEHANDLER_H

#include "macros.h"
#include "BinaryVectors.h"
#include "exceptions.h"

#include <fstream>
#include <string>

static const char * openFileError = "Error opening file: ";

class FileHandler {
public:
  FileHandler(const char * filename, std::ios_base::openmode mode) {
    file.open(filename, mode );
    if (!file) throw WrongFileException(std::string(openFileError).append(filename));
  }
  FileHandler(const std::string &filename, std::ios_base::openmode mode) {
    file.open(filename.c_str(), mode );
    if (!file) throw WrongFileException(std::string(openFileError).append(filename));
  }
  ~FileHandler() { file.close(); }
  
protected:
  std::fstream file;
};

class BinLoader : public FileHandler {
public:
  BinLoader(const char * filename)
    : FileHandler(filename, std::ios_base::in | std::ios_base::binary) { }
//  virtual ~BinLoader() { }

  Bloat getBinFileContent();
};

class BinWriter : public FileHandler {
public:
  BinWriter(const char * filename)
    : FileHandler(filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc)
  { }
  BinWriter(const std::string & filename)
    : FileHandler(filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc)
  { }
//  virtual ~BinWriter() { }

  void saveBinFileContent(const Bloat& bloat);
};

class TextLoader : public FileHandler {
public:
  TextLoader(const char * filename)
    : FileHandler(filename, std::ios_base::in) { }
  TextLoader(const std::string & filename)
    : FileHandler(filename, std::ios_base::in) { }
//  virtual ~TextLoader() { }

  std::string getTextFileContent();
};

class TextWriter : public FileHandler {
public:
  TextWriter(const char * filename)
      : FileHandler(filename, std::ios_base::out | std::ios_base::trunc ) { }
  TextWriter(const std::string & filename)
      : FileHandler(filename.c_str(), std::ios_base::out | std::ios_base::trunc ) { }

  void writeLineToFile(const std::string & line) {
    file << (line + "\n");
  }
  template<typename Type>
  TextWriter & operator<<(const Type & input) {
    file << input;
    return *this;
  }
  TextWriter & operator<<(std::ostream & (*fp)(std::ostream &)) {
    file << fp;
    return *this;
  }
  TextWriter & operator<<(std::ios & (*fp)(std::ios &)) {
    file << fp;
    return *this;
  }
  TextWriter & operator<<(std::ios_base & (*fp)(std::ios_base&)) {
    file << fp;
    return *this;
  }
};

#endif	/* _FILEHANDLER_H */

