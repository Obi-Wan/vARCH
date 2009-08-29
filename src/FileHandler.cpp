/* 
 * File:   FileHandler.cpp
 * Author: ben
 * 
 * Created on 26 agosto 2009, 18.38
 */

#include <unistd.h>

#include "FileHandler.h"


Bloat
BinLoader::getBinFileContent() {
  Bloat bloat;
  int temp = 0;
  
  while ( !file.eof() ) {
    file.read((char *)&temp, sizeof(int));
    bloat.push_back(temp);
    printf("int: %d\n",temp);
  }

  return bloat;
}

string
TextLoader::getTextFileContent() {
  string str, line;
  
  while ( !file.eof() ) {
    getline(file, line);
    str.append(line += '\n');
  }

  return str;
}

void
BinWriter::saveBinFileContent(const Bloat& bloat) {
  for(int i = 0; i < bloat.size(); i++) {
    file.write((const char *)&bloat[i], sizeof(Bloat::value_type));
  }
}