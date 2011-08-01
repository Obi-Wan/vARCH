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

#endif	/* _ASM_HELPERS_H */

