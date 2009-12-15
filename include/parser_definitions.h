/* 
 * File:   parser_definitions.h
 * Author: ben
 *
 * Created on 11 dicembre 2009, 0.03
 */

#ifndef _PARSER_DEFINITIONS_H
#define	_PARSER_DEFINITIONS_H

#include "asm_helpers.h"

#include <string>
#include <vector>
#include <map>
#include <sstream>

using namespace std;


typedef map<string, int> NameToValue;
typedef map<int, string> ValueToName;

static class IstructionSet {

  NameToValue nameToValue;
  ValueToName valueToName;

  const string errorMsgName;
  const string errorMsgValue;

  void assignNew(const string& name) {
    nameToValue.insert(NameToValue::value_type(name, currValue));
    valueToName.insert(ValueToName::value_type(currValue, name));
    currValue++;
  }
  void assignNew(const string& name, const int& newCurrVal) {
    currValue = newCurrVal;
    assignNew(name);
  }

  int currValue;

public:
  IstructionSet() : currValue(0)
                  , errorMsgName("No istruction called: ")
                  , errorMsgValue("No such istruction exists")
  {
    assignNew("SLEEP",N_ARGS_ZERO);
    assignNew("PUSHA");
    assignNew("POPA");
    assignNew("RET");
    assignNew("RETEX");
    assignNew("REBOOT",N_ARGS_ZERO + SYSTEM);
    assignNew("HALT");

    assignNew("NOT", N_ARGS_ONE);
    assignNew("INCR");
    assignNew("DECR");
    assignNew("COMP2");
    assignNew("LSH");
    assignNew("RSH");

    assignNew("STACK");
    assignNew("PUSH");
    assignNew("POP");
    assignNew("JSR");

    assignNew("JMP", N_ARGS_ONE + JUMP);
    assignNew("IFJ", N_ARGS_ONE + JUMP + CONDITIONAL);
    assignNew("IFNJ");
    assignNew("TCJ");
    assignNew("TZJ");
    assignNew("TOJ");
    assignNew("TNJ");
    assignNew("TSJ");

    assignNew("MOV", N_ARGS_TWO);
    assignNew("ADD");
    assignNew("MULT");
    assignNew("SUB");
    assignNew("DIV");
    assignNew("QUOT");
    assignNew("AND");
    assignNew("OR");
    assignNew("XOR");

    assignNew("MMU", N_ARGS_TWO + SYSTEM);

    assignNew("PUT", N_ARGS_TWO + COMUNICATION);
    assignNew("GET");

    assignNew("EQ", N_ARGS_TWO + CONDITIONAL);
    assignNew("LO");
    assignNew("MO");
    assignNew("LE");
    assignNew("ME");
    assignNew("NEQ");

    assignNew("BPUT", N_ARGS_THREE + COMUNICATION);
    assignNew("BGET");

    assignNew("IFEQJ", N_ARGS_THREE + JUMP + CONDITIONAL);
    assignNew("IFNEQJ");
    assignNew("IFLOJ");
    assignNew("IFMOJ");
    assignNew("IFLEJ");
    assignNew("IFMEJ");

  }

  const int& getIstr(const char * name) const {
    NameToValue::const_iterator istr = nameToValue.find(string(name));
    if (istr == nameToValue.end())
      throw WrongIstructionException(errorMsgName + name);

    return istr->second;
  }
  const int& getIstr(const string& name) const {
    NameToValue::const_iterator istr = nameToValue.find(name);
    if (istr == nameToValue.end())
      throw WrongIstructionException(errorMsgName + name);

    return istr->second;
  }
} ISet;

struct LineOfCode {
  int bytes;
  unsigned int lineNumber;
  vector<string> chunks;
};

struct Label {
  string name;
  unsigned int lineNumber;
  unsigned int byte;
  int offset;

  Label(const string& _n, const unsigned int& _l, const unsigned int& _b,
        const int& _o) :
        name(_n), lineNumber(_l), byte(_b), offset(_o) { }
};

typedef map<string, Label> Labels;
typedef map<string, int> Istructions;
typedef vector<int> Constants;
typedef vector< LineOfCode > CodeLines;

class Marker {
public:
  enum MarkersType {
    INT,
    LONG,
    CHAR,
    STRING,

    GLOBAL,
    LOCAL,
    FUNCTION,
    END,
  };

  static MarkersType getMarkerType(const string& type) {
    switch (type[1]) {
      case 'i':
        return INT;
      case 'l':
        if (!type.substr(1,type.size()-1).compare("long")) {
          return LONG;
        } else if (!type.substr(1,type.size()-1).compare("local")) {
          return LOCAL;
        }
        break;
      case 'c':
        return CHAR;
      case 's':
        return STRING;
      case 'g':
        return GLOBAL;
      case 'f':
        return FUNCTION;
      case 'e':
        return END;
      default:
        break;
    }
    throw WrongArgumentException(
                "No constant type: " + type + " ");
  }
};

#endif	/* _PARSER_DEFINITIONS_H */

