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
#include "exceptions.h"

#include <string>
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
    if (istr == nameToValue.end()) {
      throw WrongIstructionException(errorMsgName + name);
    }

    return istr->second;
  }
  const int& getIstr(const string& name) const {
    NameToValue::const_iterator istr = nameToValue.find(name);
    if (istr == nameToValue.end()) {
      throw WrongIstructionException(errorMsgName + name);
    }

    return istr->second;
  }
  const string& getIstr(const int& value) const {
    ValueToName::const_iterator istr = valueToName.find(value);
    if (istr == valueToName.end()) {
      throw WrongIstructionException(errorMsgValue);
    }

    return istr->second;
  }
} ISet;

#endif	/* _PARSER_DEFINITIONS_H */

