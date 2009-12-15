/* 
 * File:   asm_helpers.h
 * Author: ben
 *
 * Created on 23 agosto 2009, 18.23
 */

#ifndef _ASM_HELPERS_H
#define	_ASM_HELPERS_H

#define NAME_OF( x ) #x

#define ARG_1( x ) (x << 21)
#define ARG_2( x ) (x << 16)
#define ARG_3( x ) (x << 11)

#define ARG( y, x ) ( (x & 0x1f) << (21 - (5 * y)))
#define GET_ARG( y, x ) (( x >> (21 - (5 * y))) & 0x1f)

#define GET_ARG_1( x ) ((x >> 21) & 0x1f)
#define GET_ARG_2( x ) ((x >> 16) & 0x1f)
#define GET_ARG_3( x ) ((x >> 11) & 0x1f)

#define OFFSET_REGS   0x100
#define DATA_REGS     0
#define ADDR_REGS     1
#define STCK_PTRS     2
#define STATE_REG     3

enum Registers {
  REG_DATA_1 =      (OFFSET_REGS * DATA_REGS),
  REG_DATA_2,
  REG_DATA_3,
  REG_DATA_4,
  REG_DATA_5,
  REG_DATA_6,
  REG_DATA_7,
  REG_DATA_8,
  
  REG_ADDR_1 =      (OFFSET_REGS * ADDR_REGS),
  REG_ADDR_2,
  REG_ADDR_3,
  REG_ADDR_4,
  REG_ADDR_5,
  REG_ADDR_6,
  REG_ADDR_7,
  REG_ADDR_8,

  STACK_POINTER =   (OFFSET_REGS * STCK_PTRS),
  USER_STACK_POINTER,

  STATE_REGISTER =  (OFFSET_REGS * STATE_REG),
};

#endif	/* _ASM_HELPERS_H */

