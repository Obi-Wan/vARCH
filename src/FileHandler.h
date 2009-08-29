/* 
 * File:   FileHandler.h
 * Author: ben
 *
 * Created on 26 agosto 2009, 18.38
 */

#ifndef _FILEHANDLER_H
#define	_FILEHANDLER_H

#include <vector>
#include <fstream>
#include <string>
#include "../include/exceptions.h"

using namespace std;

typedef vector<int> Bloat;

class FileHandler {
public:
  FileHandler(const char * filename, ios_base::openmode mode) throw(WrongFileException) {
    file.open(filename, mode );
    if (!file) throw WrongFileException();
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
      : FileHandler(filename, ios_base::out | ios_base::binary | ios_base::trunc) { }
//  virtual ~BinWriter() { }

  void saveBinFileContent(const Bloat& bloat);
};

class TextLoader : public FileHandler {
public:
  TextLoader(const char * filename)
      : FileHandler(filename, ios_base::in) { }
//  virtual ~TextLoader() { }

  string getTextFileContent();
};

#endif	/* _FILEHANDLER_H */

