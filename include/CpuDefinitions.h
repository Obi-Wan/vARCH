/*
 * CpuDefinitions.h
 *
 *  Created on: 01/ago/2011
 *      Author: ben
 */

#ifndef CPUDEFINITIONS_H_
#define CPUDEFINITIONS_H_

#define NUM_REGS 8

// Signals bits
#define F_CARRY       (1 << 0)
#define F_OVERFLOW    (1 << 1)
#define F_ZERO        (1 << 2)
#define F_NEGATIVE    (1 << 3)
#define F_EXTEND      (1 << 4)
#define F_INT_MASK    (1 << 5)
#define F_SVISOR      (1 << 6)
#define F_TRACE       (1 << 7)

// Interrupts priority conversion macros
#define INT_PUT(x)    (( x & 0xf ) << 8)
#define INT_GET(x)    (( x >> 8) & 0xf )
// Some useful priorities
#define INT_MAX_S_PR  0xf
#define INT_MIN_S_PR  0x8
#define INT_MAX_U_PR  0x7
#define INT_MIN_U_PR  0x0

#endif /* CPUDEFINITIONS_H_ */
