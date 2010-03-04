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
#define SYSTEM        (1 << 27) /* Also old COMUNICATION */
#define FLOAT         (1 << 26)

// ARGS SECTION
#define RELATIVE_ARG  (1 << 4)

/* These bits occupy variably the positions from (1 << 25) to
 *    (1 << 23)
 *    (1 << 20)
 *    (1 << 17)
 * Varying on the number of arguments (on istructions non using them for args
 * type, they may be recycled.)
 */
enum TypeOfArgument {
  
  /* Here the (1 << 1) bit stays for ADDR, and the (1 << 0) for REG */
  COST = 0,               // 0000
  REG,                    // 0001
  ADDR,                   // 0010
  ADDR_IN_REG,            // 0011

  /* Here the (1 << 1) bit stays for INCR/DECR, and the (1 << 0) for PRE/POST */
  REG_PRE_INCR,           // 0100
  REG_PRE_DECR,           // 0101
  REG_POST_INCR,          // 0110
  REG_POST_DECR,          // 0111

  /* Here the (1 << 1) bit stays for INCR/DECR, and the (1 << 0) for PRE/POST */
  ADDR_PRE_INCR,          // 1000
  ADDR_PRE_DECR,          // 1001
  ADDR_POST_INCR,         // 1010
  ADDR_POST_DECR,         // 1011

  /* Here the (1 << 1) bit stays for INCR/DECR, and the (1 << 0) for PRE/POST */
  ADDR_IN_REG_PRE_INCR,   // 1100
  ADDR_IN_REG_PRE_DECR,   // 1101
  ADDR_IN_REG_POST_INCR,  // 1110
  ADDR_IN_REG_POST_DECR,  // 1111
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


enum StdInstructions {

  SLEEP         = N_ARGS_ZERO,
  PUSHA,
  POPA,
  RET,
  RETEX         = N_ARGS_ZERO + SYSTEM,
  REBOOT,
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

  PUT,
  GET,
  
  EQ            = N_ARGS_TWO + CONDITIONAL,
  LO,
  MO,
  LE,
  ME,
  NEQ,
  
  BPUT          = N_ARGS_THREE + SYSTEM,
  BGET,
  
  IFEQJ         = N_ARGS_THREE + JUMP + CONDITIONAL,
  IFNEQJ,
  IFLOJ,
  IFMOJ,
  IFLEJ,
  IFMEJ,

};

#endif	/* _STDISTRUCTIONS_H */

