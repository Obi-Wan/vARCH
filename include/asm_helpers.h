/* 
 * File:   asm_helpers.h
 * Author: ben
 *
 * Created on 23 agosto 2009, 18.23
 */

#ifndef _ASM_HELPERS_H
#define	_ASM_HELPERS_H

#define ARG_1( x ) (x << 22)
#define ARG_2( x ) (x << 19)
#define ARG_3( x ) (x << 16)

#define GET_ARG_1( x ) ((x >> 22) & 7)
#define GET_ARG_2( x ) ((x >> 19) & 7)
#define GET_ARG_3( x ) ((x >> 16) & 7)

#define OFFSET_REGS 256

enum Registers {
  REG_DATA_1 = 0,
  REG_DATA_2,
  REG_DATA_3,
  REG_DATA_4,
  REG_DATA_5,
  REG_DATA_6,
  REG_DATA_7,
  REG_DATA_8,
  
  REG_ADDR_1 = OFFSET_REGS,
  REG_ADDR_2,
  REG_ADDR_3,
  REG_ADDR_4,
  REG_ADDR_5,
  REG_ADDR_6,
  REG_ADDR_7,
  REG_ADDR_8,

  STACK_POINTER = OFFSET_REGS * 2,

  FRAME_POINTER = OFFSET_REGS * 3,

  STATE_REGISTER = OFFSET_REGS * 4,
};

#endif	/* _ASM_HELPERS_H */

