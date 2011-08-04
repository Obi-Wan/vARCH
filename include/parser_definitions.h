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
#include "macros.h"

#include <string>
#include <map>
#include <sstream>

using namespace std;


typedef map<string, int> NameToValue;
typedef map<int, string> ValueToName;

class DoubleCorrelationMap {
protected:
  NameToValue nameToValue;
  ValueToName valueToName;

  int32_t currValue;

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

public:
  DoubleCorrelationMap(const int32_t & _currValue, const string & _errorMsgName,
      const string & _errorMsgVal)
    : currValue(_currValue), errorMsgName(_errorMsgName)
    , errorMsgValue(_errorMsgVal)
  { }

  const int& getItem(const char * name) const {
    NameToValue::const_iterator istr = nameToValue.find(string(name));
    if (istr == nameToValue.end()) {
      throw WrongIstructionException(errorMsgName + name);
    }

    return istr->second;
  }
  const int& getItem(const string& name) const {
    NameToValue::const_iterator istr = nameToValue.find(name);
    if (istr == nameToValue.end()) {
      throw WrongIstructionException(errorMsgName + name);
    }

    return istr->second;
  }
  const string& getItem(const int& value) const {
    ValueToName::const_iterator istr = valueToName.find(value);
    if (istr == valueToName.end()) {
      throw WrongIstructionException(errorMsgValue);
    }

    return istr->second;
  }
};

static class IstructionSet : public DoubleCorrelationMap {
public:
  IstructionSet()
    : DoubleCorrelationMap(0, "No instruction called: ",
        "No such instruction value exists")
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

  const int& getIstr(const char * name) const { return getItem(name); }
  const int& getIstr(const string& name) const { return getItem(name); }
  const string& getIstr(const int& value) const { return getItem(value); }
} ISet;

static class ArgsTypeSet : public DoubleCorrelationMap {
public:
  ArgsTypeSet()
    : DoubleCorrelationMap(0, "No such type", "No such enum type arg")
  {
    assignNew("COST");
    assignNew("REG");
    assignNew("ADDR");
    assignNew("ADDR_IN_REG");

    /* Here the (1 << 1) bit stays for INCR/DECR, and the (1 << 0) for PRE/POST */
    assignNew("REG_PRE_INCR");
    assignNew("REG_PRE_DECR");
    assignNew("REG_POST_INCR");
    assignNew("REG_POST_DECR");

    /* Here the (1 << 1) bit stays for INCR/DECR, and the (1 << 0) for PRE/POST */
    assignNew("ADDR_PRE_INCR");
    assignNew("ADDR_PRE_DECR");
    assignNew("ADDR_POST_INCR");
    assignNew("ADDR_POST_DECR");

    /* Here the (1 << 1) bit stays for INCR/DECR, and the (1 << 0) for PRE/POST */
    assignNew("ADDR_IN_REG_PRE_INCR");
    assignNew("ADDR_IN_REG_PRE_DECR");
    assignNew("ADDR_IN_REG_POST_INCR");
    assignNew("ADDR_IN_REG_POST_DECR");

  }
} ATypeSet;

static class RegsTypeSet : public DoubleCorrelationMap {
public:
  RegsTypeSet()
    : DoubleCorrelationMap(0, "No such type", "No such enum type arg")
  {
    assignNew("REG_DATA_1", NUM_REGS * DATA_REGS);
    assignNew("REG_DATA_2");
    assignNew("REG_DATA_3");
    assignNew("REG_DATA_4");
    assignNew("REG_DATA_5");
    assignNew("REG_DATA_6");
    assignNew("REG_DATA_7");
    assignNew("REG_DATA_8");

    assignNew("REG_ADDR_1", NUM_REGS * ADDR_REGS);
    assignNew("REG_ADDR_2");
    assignNew("REG_ADDR_3");
    assignNew("REG_ADDR_4");
    assignNew("REG_ADDR_5");
    assignNew("REG_ADDR_6");
    assignNew("REG_ADDR_7");
    assignNew("REG_ADDR_8");

    assignNew("STACK_POINTER", NUM_REGS * SPECIAL_REGS);
    assignNew("USER_STACK_POINTER");

    assignNew("STATE_REGISTER");
  }
} RTypeSet;

#endif	/* _PARSER_DEFINITIONS_H */

