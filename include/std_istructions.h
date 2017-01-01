/* 
 * File:   StdIstructions.h
 * Author: ben
 *
 * Created on 19 agosto 2009, 18.36
 */

#ifndef _STDISTRUCTIONS_H
#define	_STDISTRUCTIONS_H

#include "CpuDefinitions.h"
#include "macros.h"

#define N_ARGS_ZERO   0
#define N_ARGS_ONE    (1u << 30)
#define N_ARGS_TWO    (2u << 30)
#define N_ARGS_THREE  (3u << 30)

#define GET_NUM_ARGS( x )  (((x) >> 30) & 3)

#define JUMP          (1u << 29)
#define CONDITIONAL   (1u << 28)
#define SYSTEM        (1u << 27) /* Also old COMUNICATION */
#define FLOAT         (1u << 26)

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

enum TypeOfArgument : uint8_t {
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

#define GET_BIG_DISPL(arg)  (int32_t(arg) >> 8)
#define GET_INDEX_DISPL(arg)  (int32_t(arg) >> 16)
#define GET_FIRST_REG(arg)  (int32_t(arg) & ((1 << 8)-1))
#define GET_INDEX_REG(arg) ((int32_t(arg) >> 8) & ((1 << 8)-1))

#define BUILD_BIG_DISPL(arg)  (int32_t(arg) << 8)
#define BUILD_INDEX_DISPL(arg)  (int32_t(arg) << 16)
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

#define GET_ARG_MOD(x)      ((x & MASK_PRE_POST) >> 5)

#define BUILD_PRE_POST_MOD(mod) (((mod) & 7) << 5)
#define FILTER_PRE_POST(x)  ((x) & 31)

enum Registers {
  REG_DATA_0 =        0,

  STACK_POINTER =     NUM_REGS,
  USER_STACK_POINTER,
  FRAME_POINTER,

  STATE_REGISTER,
  PROGRAM_COUNTER,

  FIRST_TEMPORARY
};

union ArgumentValue {
  int32_t immed;

  struct {
    Registers reg_id : 5;
    ModifierOfArgument reg_mod : 3;
    int32_t : 24;
  } reg;

  uint32_t direct;

  struct {
    Registers reg_id : 5;
    ModifierOfArgument reg_mod : 3;
    int32_t : 24;
  } reg_indir;

  struct {
    Registers reg_id : 5;
    ModifierOfArgument reg_mod : 3;
    int32_t : 24;
  } mem_indir;

  struct {
    Registers reg_id : 5;
    ModifierOfArgument reg_mod : 3;

    int32_t disp : 24;
  } displaced;

  struct {
    Registers reg_id : 5;
    ModifierOfArgument reg_mod : 3;

    Registers indx_id : 5;
    ModifierOfArgument indx_mod : 3;

    int16_t : 16;
  } indexed;

  struct {
    Registers reg_id : 5;
    ModifierOfArgument reg_mod : 3;

    Registers indx_id : 5;
    ModifierOfArgument indx_mod : 3;

    int16_t disp : 16;
  } indx_disp;

  int8_t bytes[4];
};

////////////////////////////////////////////////////////////////////////////////
/// STD Instructions
////////////////////////////////////////////////////////////////////////////////

enum StdInstructions : uint32_t {

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

  JSR           = N_ARGS_ONE + JUMP,
  JMP,
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

enum FloatIstructions : uint32_t {
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

