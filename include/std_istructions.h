/* 
 * File:   StdIstructions.h
 * Author: ben
 *
 * Created on 19 agosto 2009, 18.36
 */

#ifndef _STDISTRUCTIONS_H
#define	_STDISTRUCTIONS_H

#include <map>
#include <string>
#include "../include/exceptions.h"

using namespace std;

#define N_ARGS_ZERO   0
#define N_ARGS_ONE    (1 << 30)
#define N_ARGS_TWO    (2 << 30)
#define N_ARGS_THREE  (3 << 30)

#define JUMP          (1 << 29)
#define CONDITIONAL   (1 << 28)
#define COMUNICATION  (1 << 27)
#define SYSTEM        (1 << 26)

/* These bits occupy variably the positions from (1 << 25) to
 *    (1 << 23)
 *    (1 << 20)
 *    (1 << 17)
 * Varying on the number of arguments (on istructions non using them for args
 * type, they may be recycled.)
 */
enum TypeOfArgument {
  
  /* Here the (1 << 1) bit stays for ADDR, and the (1 << 0) for REG */
  COST = 0,       // 000
  REG,            // 001
  ADDR,           // 010
  ADDR_IN_REG,    // 011

  /* Here the (1 << 1) bit stays for INCR/DECR, and the (1 << 0) for PRE/POST */
  REG_PRE_INCR,   // 100
  REG_PRE_DECR,   // 101
  REG_POST_INCR,  // 110
  REG_POST_DECR,  // 111
};

//enum Registers {
//  REG_R01 =   (1 <  0),
//  REG_R02 =   (1 <  1),
//  REG_R03 =   (1 <  2),
//  REG_R04 =   (1 <  3),
//  REG_R05 =   (1 <  4),
//  REG_R06 =   (1 <  5),
//  REG_R07 =   (1 <  6),
//  REG_R08 =   (1 <  7),
//  REG_A01 =   (1 <  8),
//  REG_A02 =   (1 <  9),
//  REG_A03 =   (1 < 10),
//  REG_A04 =   (1 < 11),
//  REG_A05 =   (1 < 12),
//  REG_A06 =   (1 < 13),
//  REG_A07 =   (1 < 14),
//  REG_A08 =   (1 < 15),
//
//  REG_SP  =   (1 < 16),
//  REG_USP =   (1 < 17)
//};

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

enum StdInstructions {

  SLEEP         = N_ARGS_ZERO,
  PUSHA,
  POPA,
  RET,
  REBOOT        = N_ARGS_ZERO + SYSTEM,
  HALT,

  NOT           = N_ARGS_ONE,
  INCR,
  DECR,
  COMP2,
  LSH,
  RSH,

  STACK,
  PUSH,
  POP,
  JSR,

  JMP           = N_ARGS_ONE + JUMP,
  IFJ           = N_ARGS_ONE + JUMP + CONDITIONAL,
  IFNJ,
  TCJ,
  TZJ,
  TOJ,
  TNJ,
  TSJ,

  MOV           = N_ARGS_TWO,
  ADD,
  MULT,
  SUB,
  DIV,
  QUOT,
  AND,
  OR,
  XOR,

  MMU           = N_ARGS_TWO + SYSTEM,

  PUT           = N_ARGS_TWO + COMUNICATION,
  GET,
  
  EQ            = N_ARGS_TWO + CONDITIONAL,
  LO,
  MO,
  LE,
  ME,
  NEQ,
  
  BPUT          = N_ARGS_THREE + COMUNICATION,
  BGET,
  
  IFEQJ         = N_ARGS_THREE + JUMP + CONDITIONAL,
  IFNEQJ,
  IFLOJ,
  IFMOJ,
  IFLEJ,
  IFMEJ,

};

#define WRONG_ARG 0xFFFFFFFF

#endif	/* _STDISTRUCTIONS_H */

