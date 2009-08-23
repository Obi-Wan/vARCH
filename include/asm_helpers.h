/* 
 * File:   asm_helpers.h
 * Author: ben
 *
 * Created on 23 agosto 2009, 18.23
 */

#ifndef _ASM_HELPERS_H
#define	_ASM_HELPERS_H

#define ARG_1( x ) (x << 23)
#define ARG_2( x ) (x << 20)
#define ARG_3( x ) (x << 17)

#define OFFSET_ADDR_REGS 255

enum Registers {
  REG_DATA_1 = 0,
  REG_DATA_2,
  REG_DATA_3,
  REG_DATA_4,
  REG_DATA_5,
  REG_DATA_6,
  REG_DATA_7,
  REG_DATA_8,
  
  REG_ADDR_1 = OFFSET_ADDR_REGS,
  REG_ADDR_2,
  REG_ADDR_3,
  REG_ADDR_4,
  REG_ADDR_5,
  REG_ADDR_6,
  REG_ADDR_7,
  REG_ADDR_8,
};

#endif	/* _ASM_HELPERS_H */

