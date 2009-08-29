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
#include "../include/exceptions.h"

using namespace std;

typedef vector<int> Bloat;

class FileHandler {
public:
  FileHandler(const char * filename, ios_base::openmode mode) throw(WrongFileException) {
    if (!file) throw WrongFileException();
    file.open(filename, mode | ios_base::binary);
  }
  virtual ~FileHandler() { file.close(); }

  Bloat getFileContent();
private:
  fstream file;
};

#endif	/* _FILEHANDLER_H */

