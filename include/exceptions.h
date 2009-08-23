/* 
 * File:   exceptions.h
 * Author: ben
 *
 * Created on 23 agosto 2009, 15.42
 */

#ifndef _EXCEPTIONS_H
#define	_EXCEPTIONS_H

#include <exception>

class WrongIstructionException : public std::exception {
  const char * message;
public:
  WrongIstructionException() { message = "\0"; }
  WrongIstructionException(const char * _mess) { message = _mess; }

  const char * what() { return message; }
};

class WrongArgumentException : public WrongIstructionException {
public:
  WrongArgumentException() { }
  WrongArgumentException(const char * _mess) : WrongIstructionException(_mess) { }
};

#endif	/* _EXCEPTIONS_H */

