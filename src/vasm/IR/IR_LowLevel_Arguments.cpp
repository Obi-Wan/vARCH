/*
 * IR_LowLevel_Arguments.cpp
 *
 *  Created on: 10/lug/2012
 *      Author: ben
 */

#include "IR_LowLevel_Arguments.h"

#include "masks.h"
#include "exceptions.h"

const size_t
asm_immediate_arg::getSize() const
{
  switch (this->type) {
    case IMMED: {
      return (1 << ((size_t)this->scale));
    }
    case REG:
    case REG_INDIR:
    case MEM_INDIR:
    case DIRECT:
    case INDEXED:
    case DISPLACED:
    case INDX_DISP: {
      return 4;
    }
    default:
      throw WrongArgumentException(string(__PRETTY_FUNCTION__) + ": no such Argument type");
  }
}

void
asm_immediate_arg::emitCode(Bloat::iterator & codeIt) const
{
  if (type == IMMED) {
    switch (this->scale) {
      case BYTE1: {
        *(codeIt++) = this->content.val & SWORD;
        break;
      }
      case BYTE2: {
        *(codeIt++) = EXTRACT_LOWER__SWORD_FROM_DWORD(this->content.val);
        *(codeIt++) = EXTRACT_HIGHER_SWORD_FROM_DWORD(this->content.val);
        break;
      }
      case BYTE4: {
        const int8_t chunks[4] = DEAL_SWORDS_FROM_QWORD(this->content.val);
        for(size_t count = 0; count < 4; count++) {
          *(codeIt++) = chunks[count];
        }
        break;
      }
      case BYTE8: {
        // XXX Be careful that right now there is no correct handling of this
        const int8_t chunks[8] = DEAL_SWORDS_FROM_OWORD(this->content.lval);
        for(size_t count = 0; count < 8; count++) {
          *(codeIt++) = chunks[count];
        }
        break;
      }
      default:
        throw WrongArgumentException(string(__PRETTY_FUNCTION__) +
            "No such kind of scale for IMMED argument");
    }
  } else {
    int32_t tempVal = 0;
    switch (type) {
      case REG:
      case REG_INDIR:
      case MEM_INDIR: {
        tempVal = content.regNum + BUILD_PRE_POST_MOD(regModType);
        break;
      }
      case DIRECT: {
        // Because it is an unsigned
        tempVal = this->content.tempUID;
        break;
      }
      case DISPLACED: {
        tempVal = BUILD_BIG_DISPL(displacement)
            + BUILD_FIRST_REG(content.regNum + BUILD_PRE_POST_MOD(regModType));
        break;
      }
      case INDEXED: {
        tempVal = BUILD_INDEX_REG(index + BUILD_PRE_POST_MOD(regModType))
            + BUILD_FIRST_REG(content.regNum);
        break;
      }
      case INDX_DISP: {
        tempVal = BUILD_INDEX_DISPL(displacement)
            + BUILD_INDEX_REG(index + BUILD_PRE_POST_MOD(regModType))
            + BUILD_FIRST_REG(content.regNum);
        break;
      }
      default: {
        throw WrongArgumentException("getCode() No such kind of argument");
      }
    }
    const int8_t chunks[4] = DEAL_SWORDS_FROM_QWORD(tempVal);
    for(size_t count = 0; count < 4; count++) {
      *(codeIt++) = chunks[count];
    }
  }
}

