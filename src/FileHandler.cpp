/* 
 * File:   FileHandler.cpp
 * Author: ben
 * 
 * Created on 26 agosto 2009, 18.38
 */

#include "FileHandler.h"
#include "macros.h"

#include <unistd.h>

#include <sstream>

Bloat
BinLoader::getBinFileContent() {
  Bloat bloat;
  int temp = 0;
  
  while ( file.read((char *)&temp, sizeof(int)) ) {
    bloat.push_back(temp);
//    DebugPrintf(("int: %d\n",temp));
  }

  return bloat;
}

string
TextLoader::getTextFileContent() {
  char line[256];
  stringstream stream;
  while (file.getline(line, 256)) {
    stream << line << endl;
  }
  return stream.str();
}

void
BinWriter::saveBinFileContent(const Bloat& bloat) {
  for(int i = 0; i < bloat.size(); i++) {
    file.write((const char *)&bloat[i], sizeof(Bloat::value_type));
//    DebugPrintf(("int: %d\n",bloat[i]));
  }
}
