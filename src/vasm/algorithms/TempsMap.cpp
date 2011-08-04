/*
 * TempsMap.cpp
 *
 *  Created on: 19/lug/2011
 *      Author: ben
 */

#include "TempsMap.h"

#include "exceptions.h"
#include "CpuDefinitions.h"
#include "std_istructions.h"

#include <iostream>
#include <sstream>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
/// Class TempsMap
///
/// Public Members
////////////////////////////////////////////////////////////////////////////////

void
TempsMap::putTemp(const string & label, const uint32_t & uid,
    const bool & ignoreDups)
{
  if (!ignoreDups) {
    if (labelToUID.count(label)) {
      throw DuplicateLabelException("Temporary with label " + label +
          " already existing");
    }
    if (uidToLabel.count(uid)) {
      throw DuplicateLabelException("Temporary with uid associated to label " +
          label + " already existing");
    }
  }

  labelToUID.insert(LabelToUID::value_type(label, uid));
  uidToLabel.insert(UIDToLabel::value_type(uid, label));
}

void
TempsMap::putTemp(const uint32_t & uid, const bool & ignoreDups)
{
  stringstream stream;
  stream.fill('0');
  switch (uid) {
    case 1 ... 8: {
      stream << "R";
      stream.width(2);
      stream << uid;
      break;
    }
    case 9 ... 16: {
      stream << "A";
      stream.width(2);
      stream << uid;
      break;
    }
    case STACK_POINTER+1: {
      stream << "SP";
      break;
    }
    case USER_STACK_POINTER+1: {
      stream << "USP";
      break;
    }
    case STATE_REGISTER+1: {
      stream << "SR";
      break;
    }
    default: {
      stream << "T";
      stream.width(10);
      stream << uid;
      break;
    }
  }
  putTemp(stream.str(), uid, ignoreDups);
}

void
TempsMap::clear()
{
  uidToLabel.clear();
  labelToUID.clear();
}

const string &
TempsMap::getLabel(const uint32_t & uid) const
{
  UIDToLabel::const_iterator label = uidToLabel.find(uid);
  if (label == uidToLabel.end()) {
    stringstream stream;
    stream << "UID: " << uid << " not in Temporary List";
    throw WrongArgumentException(stream.str());
  }
  return label->second;
}

const uint32_t &
TempsMap::getUID(const string & label) const
{
  LabelToUID::const_iterator uid = labelToUID.find(label);
  if (uid == labelToUID.end()) {
    throw WrongArgumentException("Label " + label + " not in Temporary List");
  }
  return uid->second;
}
