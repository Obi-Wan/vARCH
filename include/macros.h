/* 
 * File:   macros.h
 * Author: ben
 *
 * Created on 29 agosto 2009, 20.17
 */

#ifndef _MACROS_H
#define	_MACROS_H

#if defined(HAVE_CONFIG_H)
/* Autotools build */
# include "config.h"
#elif defined(SCONS_BUILD)
/* SCons Build */
# if defined(NDEBUG) && defined(DEBUG)
#   undef NDEBUG
# endif
#elif defined(_MSC_VER)
# if defined(_DEBUG)
#   define DEBUG
# endif
#else
/* Other, potentially unsupported building system */
# warning "Unknown building system, may cause unexpected behavior"
#endif

/* Common MSVC props */
#if defined(_MSC_VER)
# pragma warning(disable: 4290)
#endif

/* Message Reporting Level */

#ifdef DEBUG
# ifndef WARNING
#   define WARNING
# endif
#endif

#ifdef WARNING
# ifndef INFO
#   define INFO
# endif
#endif

#if defined(DEBUG) || defined(WARNING) || defined(INFO)
# include <cstdio>
# include <iostream>
using namespace std;

# ifndef __PRETTY_FUNCTION__
#   define __PRETTY_FUNCTION__ __FUNCTION__
# endif
#endif

/* ********* Macros Definitions for Programmer's Use ***************************
 * If the condition is false, it throws the exception
 */
#define CHECK_THROW( condition, exception ) do {\
  if (!(condition)) {\
    throw exception;\
  }\
} while (0)

#ifdef DEBUG
# define INLINE
# define DebugPrintf( x ) do { printf("Debug: "); printf x; } while(0)
# define DebugReportException( x ) printf("Debug: %s", x.what())
# define DebugPrintfMaps( mapType, mapObj, mapName ) do { \
    cout << "DEBUG: " << mapName << "\n"; \
    for(mapType::const_iterator i = mapObj.begin(); i != mapObj.end(); i++) { \
      cout << "first: \"" << i->first << "\", second: " << i->second << "\n"; \
    }\
  } while(0)
# define DebugPrintfLabels( mapObj, mapName ) do { \
    cout << "DEBUG: " << mapName << "\n"; \
    for(Labels::const_iterator i = mapObj.begin(); i != mapObj.end(); i++) { \
      cout << "name: \"" << i->first << "\", line: " << i->second.lineNumber \
           << " bytePos: " << i->second.byte << "\n"; \
    }\
  } while(0)
# define DebugPrintfCodeLines( obj, name ) do { \
    cout << "DEBUG: " << name << "\n"; \
    for(CodeLines::const_iterator i = obj.begin(); i != obj.end(); i++) { \
      cout << "Line: " << i->lineNumber << ", segments: "; \
      for(vector<string>::const_iterator j = i->chunks.begin(); \
          j != i->chunks.end(); j++) { \
        cout << *j << " "; \
      } \
      cout << "\nnumber of bytes: " << i->bytes << "\n"; \
    }\
  } while(0)
#else
# define INLINE inline
# define DebugPrintf( x )
# define DebugReportException( x )
# define DebugPrintfLabels( mapObj, mapName )
# define DebugPrintfMaps( mapType, mapObj, mapName )
# define DebugPrintfCodeLines( obj, name )
#endif

#ifdef WARNING
# define WarningPrintf( x ) do { printf("Warning: "); printf x; } while(0)
# define WarningReportException( x ) printf("Warning: %s", x.what())
#else
# define WarningPrintf( x )
# define WarningReportException( x )
#endif

#ifdef INFO
# define InfoPrintf( x ) do { printf("Info: "); printf x; } while(0)
# define InfoReportException( x ) printf("Info: %s", x.what())
#else
# define InfoPrintf( x )
# define InfoReportException( x )
#endif

#endif	/* _MACROS_H */

