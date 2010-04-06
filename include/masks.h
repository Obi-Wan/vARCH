/* 
 * File:   masks.h
 * Author: ben
 *
 * Created on 3 marzo 2010, 15.40
 */

#ifndef _MASKS_H
#define	_MASKS_H

#define SWORD 0x00000000000000ff
#define DWORD 0x000000000000ffff
#define TWORD 0x0000000000ffffff
#define QWORD 0x00000000ffffffff
#define OWORD 0xffffffffffffffff

/* Operations on DWORDS */
#define EXTRACT_HIGHER_SWORD_FROM_DWORD(x) ((x >>  8) & SWORD)
#define EXTRACT_LOWER__SWORD_FROM_DWORD(x) ((x      ) & SWORD)

/* Operations on QWORDS */
#define EXTRACT_HIGHER_SWORD_FROM_QWORD(x) ((x >> 24) & SWORD)
#define EXTRACT_HIGHER_DWORD_FROM_QWORD(x) ((x >> 16) & DWORD)
#define EXTRACT_HIGHER_TWORD_FROM_QWORD(x) ((x >>  8) & TWORD)
#define EXTRACT_LOWER__SWORD_FROM_QWORD(x) ((x      ) & SWORD)
#define EXTRACT_LOWER__DWORD_FROM_QWORD(x) ((x      ) & DWORD)
#define EXTRACT_LOWER__TWORD_FROM_QWORD(x) ((x      ) & TWORD)

/* Operations on OWORDS */
#define EXTRACT_HIGHER_QWORD_FROM_OWORD(x) ((x >> 32) & QWORD)
#define EXTRACT_LOWER__QWORD_FROM_OWORD(x) ((x      ) & QWORD)

#endif	/* _MASKS_H */
