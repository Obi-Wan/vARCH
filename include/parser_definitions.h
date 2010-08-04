/* 
 * File:   parser_definitions.h
 * Author: ben
 *
 * Created on 11 dicembre 2009, 0.03
 */

#ifndef _PARSER_DEFINITIONS_H
#define	_PARSER_DEFINITIONS_H

#include "asm_helpers.h"
#include "std_istructions.h"

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
    assignNew("RETEX",N_ARGS_ZERO + SYSTEM);
    assignNew("REBOOT");
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

    assignNew("PUT");
    assignNew("GET");

    assignNew("EQ", N_ARGS_TWO + CONDITIONAL);
    assignNew("LO");
    assignNew("MO");
    assignNew("LE");
    assignNew("ME");
    assignNew("NEQ");

    assignNew("BPUT", N_ARGS_THREE + SYSTEM);
    assignNew("BGET");

    assignNew("IFEQJ", N_ARGS_THREE + JUMP + CONDITIONAL);
    assignNew("IFNEQJ");
    assignNew("IFLOJ");
    assignNew("IFMOJ");
    assignNew("IFLEJ");
    assignNew("IFMEJ");

    /* Float instructions */
    assignNew("FNOT", N_ARGS_ONE + FLOAT);
    assignNew("FINCR");
    assignNew("FDECR");

    assignNew("FMOV", N_ARGS_TWO + FLOAT);
    assignNew("FADD");
    assignNew("FMULT");
    assignNew("FSUB");
    assignNew("FDIV");
    assignNew("FQUOT");

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

  static MarkersType getMarkerType(const string& type) throw(WrongArgumentException) {
    switch (type[1]) {
      case 'i':
	if (!type.compare(".int")) return INT;
	else break;
      case 'l':
        if (!type.compare(".long")) {
          return LONG;
        } else if (!type.compare(".local")) {
          return LOCAL;
        } else {
          break;
	}
      case 'c':
	if (!type.compare(".char")) return CHAR;
	else break;
      case 's':
	if (!type.compare(".string")) return STRING;
	else break;
      case 'g':
	if (!type.compare(".global")) return GLOBAL;
	else break;
      case 'f':
	if (!type.compare(".function")) return FUNCTION;
	else break;
      case 'e':
	if (!type.compare(".end")) return END;
	else break;
      default:
        break;
    }
    throw WrongArgumentException(
                "No constant type: \"" + type + "\"");
  }
};

#endif	/* _PARSER_DEFINITIONS_H */

