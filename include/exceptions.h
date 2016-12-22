/* 
 * File:   exceptions.h
 * Author: ben
 *
 * Created on 23 agosto 2009, 15.42
 */

#ifndef _EXCEPTIONS_H
#define	_EXCEPTIONS_H

#include <stdexcept>
#include <string>

class BasicException : public std::exception {
  std::string message;
public:
  BasicException() = default;
  BasicException(const char * _mess) : message(_mess) { }
  BasicException(const std::string& _mess) : message(_mess) { }
  BasicException(const BasicException& _ex) : message(_ex.message) { }

  virtual ~BasicException() throw() = default;

  virtual const char * what() const throw() { return message.c_str(); }
  const std::string& getMessage() const throw() { return message; }

  virtual void setMessage(const std::string & _mess) { message = _mess; }
  virtual void appendMessage(const std::string & _mess) { message += _mess; }
  virtual void prefixMessage(const std::string & _mess) { message = _mess + message; }
};

class MmuException : public BasicException {
public:
  MmuException() = default;
  MmuException(const char * _mess) : BasicException(_mess) { }
  MmuException(const std::string& _mess) : BasicException(_mess) { }
};

class WrongComponentException : public BasicException {
public:
  WrongComponentException() = default;
  WrongComponentException(const char * _mess) : BasicException(_mess) { }
  WrongComponentException(const std::string & _mess) : BasicException(_mess) { }
};

class WrongInstructionException : public BasicException {
public:
  WrongInstructionException() = default;
  WrongInstructionException(const char * _mess) : BasicException(_mess) { }
  WrongInstructionException(const std::string & _mess) : BasicException(_mess) { }
};

class WrongArgumentException : public WrongInstructionException {
public:
  WrongArgumentException() = default;
  WrongArgumentException(const char * _mess) : WrongInstructionException(_mess) { }
  WrongArgumentException(const std::string & _mess) : WrongInstructionException(_mess) { }
};

class WrongFileException : public BasicException {
public:
  WrongFileException() = default;
  WrongFileException(const char * _mess) : BasicException(_mess) { }
  WrongFileException(const std::string & _mess) : BasicException(_mess) { }
};

class DuplicateLabelException : public BasicException {
public:
  DuplicateLabelException() = default;
  DuplicateLabelException(const char * _mess) : BasicException(_mess) { }
  DuplicateLabelException(const std::string & _mess) : BasicException(_mess) { }
};

//class DuplicateConstException : public BasicException {
//public:
//  DuplicateConstException() = default;
//  DuplicateConstException(const char * _mess) : BasicException(_mess) { }
//  DuplicateConstException(const std::string & _mess) : BasicException(_mess) { }
//};

#endif	/* _EXCEPTIONS_H */

