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

#include <vector>
#include <fstream>
#include <string>
#include "exceptions.h"

using namespace std;

static const char * openFileError = "Error opening file: ";

class FileHandler {
public:
  FileHandler(const char * filename, ios_base::openmode mode) {
    file.open(filename, mode );
    if (!file) throw WrongFileException(string(openFileError).append(filename));
  }
  FileHandler(const string &filename, ios_base::openmode mode) {
    file.open(filename.c_str(), mode );
    if (!file) throw WrongFileException(string(openFileError).append(filename));
  }
  ~FileHandler() { file.close(); }
  
protected:
  fstream file;
};

class BinLoader : public FileHandler {
public:
  BinLoader(const char * filename)
    : FileHandler(filename, ios_base::in | ios_base::binary) { }
//  virtual ~BinLoader() { }

  Bloat getBinFileContent();
};

class BinWriter : public FileHandler {
public:
  BinWriter(const char * filename)
    : FileHandler(filename, ios_base::out | ios_base::binary | ios_base::trunc)
  { }
  BinWriter(const string & filename)
    : FileHandler(filename, ios_base::out | ios_base::binary | ios_base::trunc)
  { }
//  virtual ~BinWriter() { }

  void saveBinFileContent(const Bloat& bloat);
};

class TextLoader : public FileHandler {
public:
  TextLoader(const char * filename)
    : FileHandler(filename, ios_base::in) { }
  TextLoader(const string & filename)
    : FileHandler(filename, ios_base::in) { }
//  virtual ~TextLoader() { }

  string getTextFileContent();
};

class TextWriter : public FileHandler {
public:
  TextWriter(const char * filename)
      : FileHandler(filename, ios_base::out | ios_base::trunc ) { }
  TextWriter(const string & filename)
      : FileHandler(filename.c_str(), ios_base::out | ios_base::trunc ) { }

  void writeLineToFile(const string & line) {
    file << (line + "\n");
  }
  template<typename Type>
  TextWriter & operator<<(const Type & input) {
    file << input;
    return *this;
  }
  TextWriter & operator<<(ostream & (*fp)(ostream &)) {
    file << fp;
    return *this;
  }
  TextWriter & operator<<(ios & (*fp)(ios&)) {
    file << fp;
    return *this;
  }
  TextWriter & operator<<(ios_base & (*fp)(ios_base&)) {
    file << fp;
    return *this;
  }
};

#endif	/* _FILEHANDLER_H */

