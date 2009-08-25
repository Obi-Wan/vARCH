/* 
 * File:   StdIstructions.h
 * Author: ben
 *
 * Created on 19 agosto 2009, 18.36
 */

#ifndef _STDISTRUCTIONS_H
#define	_STDISTRUCTIONS_H

#define N_ARGS_ZERO   0
#define N_ARGS_ONE    (1 << 30)
#define N_ARGS_TWO    (2 << 30)
#define N_ARGS_THREE  (3 << 30)

#define ALGEBRIC      (1 << 29)
#define JUMP          (1 << 28)
#define CONDITIONAL   (1 << 27)
#define COMUNICATION  (1 << 26)
#define SYSTEM        (1 << 25)

/* These bits occupy variably the positions from (1 << 24) to
 *    (1 << 22)
 *    (1 << 19)
 *    (1 << 16)
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

