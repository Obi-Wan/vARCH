/* 
 * File:   macros.h
 * Author: ben
 *
 * Created on 29 agosto 2009, 20.17
 */

#ifndef _MACROS_H
#define	_MACROS_H

//#define DEBUG

#ifdef DEBUG
  #define WARNING
#include <iostream>
#include <cstdio>

using namespace std;

#define DebugPrintf( x ) do { printf x; } while(0)
#define DebugPrintfMaps( mapType, mapObj, mapName ) do { \
  cout << "DEBUG: " << mapName << "\n"; \
  for(mapType::iterator i = mapObj.begin(); i != mapObj.end(); i++) { \
    cout << "first: \"" << i->first << "\", second: " << i->second << "\n"; \
  }\
} while(0)
#else
#define DebugPrintf( x )
#define DebugPrintfMaps( mapType, mapObj, mapName )
#endif

#define WARNING

#ifdef WARNING
#include <cstdio>

using namespace std;

#define WarningPrintf( x ) do { printf("Warning: "); printf x; } while(0)
#else
#define WarningPrintf( x )
#endif

#endif	/* _MACROS_H */

