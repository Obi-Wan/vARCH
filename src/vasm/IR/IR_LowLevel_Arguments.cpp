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
        *(codeIt++) = this->content.val & BWORD;
        break;
      }
      case BYTE2: {
        *(codeIt++) = EXTRACT_LOWER__BWORD_FROM_HWORD(this->content.val);
        *(codeIt++) = EXTRACT_HIGHER_BWORD_FROM_HWORD(this->content.val);
        break;
      }
      case BYTE4: {
        const int8_t chunks[4] = DEAL_BWORDS_FROM_SWORD(this->content.val);
        for(size_t count = 0; count < 4; count++) {
          *(codeIt++) = chunks[count];
        }
        break;
      }
      case BYTE8: {
        // XXX Be careful that right now there is no correct handling of this
        const int8_t chunks[8] = DEAL_BWORDS_FROM_DWORD(this->content.val);
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
    ArgumentValue tempVal;
    switch (type) {
      case REG: {
        tempVal.reg.reg_id = content.regNum;
        tempVal.reg.reg_mod = regModType;
        break;
      }
      case REG_INDIR: {
        tempVal.reg_indir.reg_id = content.regNum;
        tempVal.reg_indir.reg_mod = regModType;
        break;
      }
      case MEM_INDIR: {
        tempVal.mem_indir.reg_id = content.regNum;
        tempVal.mem_indir.reg_mod = regModType;
        break;
      }
      case DIRECT: {
        // Because it is an unsigned
        tempVal.direct = content.tempUID;
        break;
      }
      case DISPLACED: {
        tempVal.displaced.disp = displacement;
        tempVal.displaced.reg_id = content.regNum;
        tempVal.displaced.reg_mod = regModType;
        break;
      }
      case INDEXED: {
        tempVal.indexed.reg_id = content.regNum;
        tempVal.indexed.indx_id = (Registers)index;
        tempVal.indexed.indx_mod = regModType;
        break;
      }
      case INDX_DISP: {
        tempVal.indx_disp.disp = displacement;
        tempVal.indx_disp.reg_id = content.regNum;
        tempVal.indx_disp.indx_id = (Registers)index;
        tempVal.indx_disp.indx_mod = regModType;
        break;
      }
      default: {
        throw WrongArgumentException(string(__PRETTY_FUNCTION__)
            + " No such kind of argument: " + to_string(type));
      }
    }
    for(size_t count = 0; count < 4; count++) {
      *(codeIt++) = tempVal.bytes[count];
    }
  }
}

