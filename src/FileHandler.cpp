/* 
 * File:   FileHandler.cpp
 * Author: ben
 * 
 * Created on 26 agosto 2009, 18.38
 */

#include <unistd.h>

#include "FileHandler.h"


Bloat
FileHandler::getFileContent() {
  Bloat bloat;
  int temp = 0;
  
  while ( file.read((char *)temp, sizeof(temp)) ) {
    bloat.push_back(temp);
  }

  return bloat;
}
