/* 
 * File:   exceptions.h
 * Author: ben
 *
 * Created on 23 agosto 2009, 15.42
 */

#ifndef _EXCEPTIONS_H
#define	_EXCEPTIONS_H

#include <exception>
#include <string>

using namespace std;

class BasicException : public exception {
  string message;
public:
  BasicException() { }
  BasicException(const char * _mess) : message(_mess) { }
  BasicException(const string& _mess) : message(_mess) { }
  BasicException(const BasicException& _ex) : message(_ex.message) { }

  virtual ~BasicException() throw() { }

  virtual const char * what() const throw() { return message.data(); }
  const string& getMessage() const throw() { return message; }
};

class WrongIstructionException : public BasicException {
public:
  WrongIstructionException() { }
  WrongIstructionException(const char * _mess) : BasicException(_mess) { }
  WrongIstructionException(const string& _mess) : BasicException(_mess) { }
};

class WrongArgumentException : public WrongIstructionException {
public:
  WrongArgumentException() { }
  WrongArgumentException(const char * _mess) : WrongIstructionException(_mess) { }
  WrongArgumentException(const string& _mess) : WrongIstructionException(_mess) { }
};

class WrongFileException : public BasicException {
public:
  WrongFileException() { }
  WrongFileException(const char * _mess) : BasicException(_mess) { }
  WrongFileException(const string& _mess) : BasicException(_mess) { }
};

#endif	/* _EXCEPTIONS_H */

