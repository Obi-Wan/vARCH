/* 
 * File:   masks.h
 * Author: ben
 *
 * Created on 3 marzo 2010, 15.40
 */

#ifndef _MASKS_H
#define	_MASKS_H

#include "macros.h"

#define BWORD 0x00000000000000ff
#define HWORD 0x000000000000ffff
#define SWORD 0x00000000ffffffff
#define DWORD 0xffffffffffffffff

/* Operations on HWORDS */
#define EXTRACT_HIGHER_BWORD_FROM_HWORD(x) ((x >>  8) & BWORD)
#define EXTRACT_LOWER__BWORD_FROM_HWORD(x) ((x      ) & BWORD)

/* Operations on SWORDS */
#define EXTRACT_HIGHER_BWORD_FROM_SWORD(x) ((x >> 24) & BWORD)
#define EXTRACT_HIGHER_HWORD_FROM_SWORD(x) ((x >> 16) & HWORD)
#define EXTRACT_LOWER__BWORD_FROM_SWORD(x) ((x      ) & BWORD)
#define EXTRACT_LOWER__HWORD_FROM_SWORD(x) ((x      ) & HWORD)

#define DEAL_BWORDS_FROM_SWORD(x) \
  { static_cast<int8_t>(x & BWORD), static_cast<int8_t>((x >>  8) & BWORD), \
    static_cast<int8_t>((x >> 16) & BWORD), static_cast<int8_t>((x >> 24) & BWORD) }

template<typename Type = int16_t, typename pointer>
inline Type DEAL_HWORD_FROM_BWORDS(pointer & x)
{
  Type temp = 0;
  temp += (((Type)(*x++) & BWORD) <<  0);
  temp += (((Type)(*x++) & BWORD) <<  8);
  return temp;
}

// Same as macro, but with perfectly defined behavior
//
//#define DEAL_SWORD_FROM_BWORDS(x) \
//    (((int32_t)(*x++) & BWORD) + (((int32_t)(*x++) & BWORD) << 8) \
//    + (((int32_t)(*x++) & BWORD) << 16) + (((int32_t)(*x++) & BWORD) << 24))
template<typename Type = int32_t, typename pointer>
inline Type DEAL_SWORD_FROM_BWORDS(pointer & x)
{
  union { int8_t bytes[4]; int32_t sword; } temp;
  for(size_t num = 0; num < 4; num++)
  {
    temp.bytes[num] = *x++;
  }
  return (Type)temp.sword;
}

template<typename Type = int64_t, typename pointer>
inline Type DEAL_DWORD_FROM_BWORDS(pointer & x)
{
  Type temp = 0;
  temp += (((Type)(*x++) & BWORD) <<  0);
  temp += (((Type)(*x++) & BWORD) <<  8);
  temp += (((Type)(*x++) & BWORD) << 16);
  temp += (((Type)(*x++) & BWORD) << 24);
  temp += (((Type)(*x++) & BWORD) << 32);
  temp += (((Type)(*x++) & BWORD) << 40);
  temp += (((Type)(*x++) & BWORD) << 48);
  temp += (((Type)(*x++) & BWORD) << 56);
  return temp;
}

/* Operations on DWORDS */
#define EXTRACT_HIGHER_SWORD_FROM_DWORD(x) ((x >> 32) & SWORD)
#define EXTRACT_LOWER__SWORD_FROM_DWORD(x) ((x      ) & SWORD)

#define DEAL_BWORDS_FROM_DWORD(x) \
  { static_cast<int8_t>(x & BWORD), static_cast<int8_t>((x >>  8) & BWORD), \
    static_cast<int8_t>((x >> 16) & BWORD), static_cast<int8_t>((x >> 24) & BWORD), \
    static_cast<int8_t>((x >> 32) & BWORD), static_cast<int8_t>((x >>  40) & BWORD), \
    static_cast<int8_t>((x >> 48) & BWORD), static_cast<int8_t>((x >> 56) & BWORD) }

#endif	/* _MASKS_H */

