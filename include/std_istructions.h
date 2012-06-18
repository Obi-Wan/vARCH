/* 
 * File:   StdIstructions.h
 * Author: ben
 *
 * Created on 19 agosto 2009, 18.36
 */

#ifndef _STDISTRUCTIONS_H
#define	_STDISTRUCTIONS_H

#include "CpuDefinitions.h"

#define N_ARGS_ZERO   0
#define N_ARGS_ONE    (1 << 30)
#define N_ARGS_TWO    (2 << 30)
#define N_ARGS_THREE  (3 << 30)

#define GET_NUM_ARGS( x )  (((x) >> 30) & 3)

#define JUMP          (1 << 29)
#define CONDITIONAL   (1 << 28)
#define SYSTEM        (1 << 27) /* Also old COMUNICATION */
#define FLOAT         (1 << 26)

/* ASM Helpers */
#define NAME_OF( x ) #x

#define ARG_1( x ) ((x) << 21)
#define ARG_2( x ) ((x) << 16)
#define ARG_3( x ) ((x) << 11)

#define ARG( y, x )     ( ((x) & 0x1f) << (21 - 5 * (y) ) )
#define GET_ARG( y, x ) ( ( (x) >> (21 - 5 * (y) )) & 0x1f)

#define GET_ARG_1( x ) (((x) >> 21) & 0x1f)
#define GET_ARG_2( x ) (((x) >> 16) & 0x1f)
#define GET_ARG_3( x ) (((x) >> 11) & 0x1f)
/* End ASM Helpers */

// ARGS SECTION
//#define RELATIVE_ARG  (1 << 4)
//#define IS_RELATIVE( x ) ((x & RELATIVE_ARG) == RELATIVE_ARG)

/* These bits occupy variably the positions from (1 << 25) to
 *    (1 << 23)
 *    (1 << 20)
 *    (1 << 17)
 * Varying on the number of arguments (on istructions non using them for args
 * type, they may be recycled.)
 */
//enum TypeOfArgument {
//
//  /* Here the (1 << 1) bit stays for DIRECT, and the (1 << 0) for REG */
//  IMMED = 0,              // 0000
//  REG,                    // 0001
//  DIRECT,                 // 0010
//  REG_INDIR,              // 0011
//
//  /* Here the (1 << 1) bit stays for INCR/DECR, and the (1 << 0) for PRE/POST */
//  REG_PRE_INCR,           // 0100
//  REG_PRE_DECR,           // 0101
//  REG_POST_INCR,          // 0110
//  REG_POST_DECR,          // 0111
//
//  /* Here the (1 << 1) bit stays for INCR/DECR, and the (1 << 0) for PRE/POST */
//  ADDR_PRE_INCR,          // 1000
//  ADDR_PRE_DECR,          // 1001
//  ADDR_POST_INCR,         // 1010
//  ADDR_POST_DECR,         // 1011
//
//  /* Here the (1 << 1) bit stays for INCR/DECR, and the (1 << 0) for PRE/POST */
//  ADDR_IN_REG_PRE_INCR,   // 1100
//  ADDR_IN_REG_PRE_DECR,   // 1101
//  ADDR_IN_REG_POST_INCR,  // 1110
//  ADDR_IN_REG_POST_DECR,  // 1111
//};

enum TypeOfArgument {
  /* Immediate */
  IMMED = 0,              // 000
  /* Register */
  REG,                    // 001
  /* Direct addressing */
  DIRECT,                 // 010
  /* Indirect addressing, with address in register */
  REG_INDIR,              // 011
  /* Indirect addressing, with address in memory, reached from address */
  MEM_INDIR,              // 100
  /* Displaced addressing (offset in register) */
  DISPLACED,              // 101
  /* Indexed addressing (fixed offset) */
  INDEXED,                // 110
  /* Indexed + Displaced addressing */
  INDX_DISP,              // 111
};

#define GET_BIG_DISPL(arg)  ((((arg) >> 8) & ((1 << 23)-1)) + ((arg) & (1 << 31)))
#define GET_INDEX_DISPL(arg)  ((((arg) >> 16) & ((1 << 15)-1)) + ((arg) & (1 << 31)))
#define GET_FIRST_REG(arg)  ((arg) & ((1 << 8)-1))
#define GET_INDEX_REG(arg) (((arg) >> 8) & ((1 << 8)-1))

#define BUILD_BIG_DISPL(arg)  ((((arg) & ((1 << 23)-1)) << 8) + ((arg) & (1 << 31)))
#define BUILD_INDEX_DISPL(arg)  ((((arg) & ((1 << 15)-1)) << 16) + ((arg) & (1 << 31)))
#define BUILD_FIRST_REG(arg)  ((arg) & ((1 << 8)-1))
#define BUILD_INDEX_REG(arg) (((arg) & ((1 << 8)-1)) << 8)

enum ScaleOfArgument {
  BYTE1 = 0,              // 00
  BYTE2,                  // 01
  BYTE4,                  // 10
  BYTE8,                  // 11
};

#define SCALE_ADDR_INCREM(arg, scale) arg += (1 << (scale))
#define SCALE_ADDR_DECREM(arg, scale) arg -= (1 << (scale))

#define BUILD_ARG(scale, type)  ((((scale) & 3) << 3) + ((type) & 7))
#define GET_ARG_TYPE(arg)   ((arg) & 7)
#define GET_ARG_SCALE(arg)  (((arg) >> 3) & 3)

//#define MASK_PRE_POST       (0xC)   // 1100
//#define IS_PRE_POST_MOD(x)  (((x) & MASK_PRE_POST) && true)

enum ModifierOfArgument {
  REG_NO_ACTION = 0,          // 000
  REG_PRE_INCR = (1 << 2),    // 100
  REG_PRE_DECR,               // 101
  REG_POST_INCR,              // 110
  REG_POST_DECR,              // 111
};

/* Info on pre/post incr/decr is in argument now */
#define MASK_PRE_POST       (7 << 5) // 11100000
#define IS_PRE_POST_MOD(x)  (((x) & (1 << 7)) && true)
#define IS_POST(x)          (((x) & (1 << 6)) && true)
#define IS_DECR(x)          (((x) & (1 << 5)) && true)

#define BUILD_PRE_POST_MOD(mod) (((mod) & 7) << 5)
#define FILTER_PRE_POST(x)  ((x) & 31)

#define DATA_REGS     0
#define ADDR_REGS     1
#define SPECIAL_REGS  2

enum Registers {
  REG_DATA_1 =      (NUM_REGS * DATA_REGS),
  REG_DATA_2,
  REG_DATA_3,
  REG_DATA_4,
  REG_DATA_5,
  REG_DATA_6,
  REG_DATA_7,
  REG_DATA_8,

  REG_ADDR_1 =      (NUM_REGS * ADDR_REGS),
  REG_ADDR_2,
  REG_ADDR_3,
  REG_ADDR_4,
  REG_ADDR_5,
  REG_ADDR_6,
  REG_ADDR_7,
  REG_ADDR_8,

  STACK_POINTER =   (NUM_REGS * SPECIAL_REGS),
  USER_STACK_POINTER,

  STATE_REGISTER,
  PROGRAM_COUNTER,

  FIRST_TEMPORARY
};


////////////////////////////////////////////////////////////////////////////////
/// STD Instructions
////////////////////////////////////////////////////////////////////////////////

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

enum FloatIstructions {
  FNOT           = N_ARGS_ONE + FLOAT,
  FINCR,
  FDECR,

  FMOV           = N_ARGS_TWO + FLOAT,
  FADD,
  FMULT,
  FSUB,
  FDIV,
  FQUOT,
};

#endif	/* _STDISTRUCTIONS_H */

