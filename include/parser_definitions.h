/* 
 * File:   parser_definitions.h
 * Author: ben
 *
 * Created on 11 dicembre 2009, 0.03
 */

#ifndef _PARSER_DEFINITIONS_H
#define	_PARSER_DEFINITIONS_H

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
      throw WrongInstructionException(errorMsgName + name);
    }

    return istr->second;
  }
  const int& getItem(const string& name) const {
    NameToValue::const_iterator istr = nameToValue.find(name);
    if (istr == nameToValue.end()) {
      throw WrongInstructionException(errorMsgName + name);
    }

    return istr->second;
  }
  const string& getItem(const int& value) const {
    ValueToName::const_iterator istr = valueToName.find(value);
    if (istr == valueToName.end()) {
      throw WrongInstructionException(errorMsgValue);
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

    assignNew("JSR", N_ARGS_ONE + JUMP);
    assignNew("JMP");
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

  const int& getInstr(const char * name) const { return getItem(name); }
  const int& getInstr(const string& name) const { return getItem(name); }
  const string& getInstr(const int& value) const { return getItem(value); }
} ISet;

static class ArgsTypeSet : public DoubleCorrelationMap {
public:
  ArgsTypeSet()
    : DoubleCorrelationMap(0, "No such type", "No such enum type arg")
  {
    /* Immediate */
    assignNew("IMMED", 0);               // 000
    /* Register */
    assignNew("REG");                    // 001
    /* Direct addressing */
    assignNew("DIRECT");                 // 010
    /* Indirect addressing, with address in register */
    assignNew("REG_INDIR");              // 011
    /* Indirect addressing, with address in memory, reached from address */
    assignNew("MEM_INDIR");              // 100
    /* Displaced addressing (offset in register) */
    assignNew("DISPLACED");              // 101
    /* Indexed addressing (fixed offset) */
    assignNew("INDEXED");                // 110
    /* Indexed + Displaced addressing */
    assignNew("INDX_DISP");              // 111

  }
} ATypeSet;

static class ModsTypeSet : public DoubleCorrelationMap {
public:
  ModsTypeSet()
    : DoubleCorrelationMap(0, "No such modifier", "No such enum modifier arg")
  {
    assignNew("REG_NO_ACTION", 0);       // 000
    assignNew("REG_PRE_INCR", (1 << 2)); // 100
    assignNew("REG_PRE_DECR");           // 101
    assignNew("REG_POST_INCR");          // 110
    assignNew("REG_POST_DECR");          // 111

  }
} MTypeSet;

static class ScalesTypeSet : public DoubleCorrelationMap {
public:
  ScalesTypeSet()
    : DoubleCorrelationMap(0, "No such modifier", "No such enum scale arg")
  {
    assignNew("BYTE1", 0);               // 00
    assignNew("BYTE2");                  // 01
    assignNew("BYTE4");                  // 10
    assignNew("BYTE8");                  // 11

  }
} STypeSet;

static class RegsTypeSet : public DoubleCorrelationMap {
public:
  RegsTypeSet()
    : DoubleCorrelationMap(0, "No such type", "No such enum register arg")
  {
    assignNew("R0", NUM_REGS * DATA_REGS);
    assignNew("R1");
    assignNew("R2");
    assignNew("R3");
    assignNew("R4");
    assignNew("R5");
    assignNew("R6");
    assignNew("R7");

    assignNew("A0", NUM_REGS * ADDR_REGS);
    assignNew("A1");
    assignNew("A2");
    assignNew("A3");
    assignNew("A4");
    assignNew("A5");
    assignNew("A6");
    assignNew("A7");

    assignNew("SP", NUM_REGS * SPECIAL_REGS);
    assignNew("USP");

    assignNew("SR");
    assignNew("PC");
  }
} RTypeSet;

#endif	/* _PARSER_DEFINITIONS_H */

