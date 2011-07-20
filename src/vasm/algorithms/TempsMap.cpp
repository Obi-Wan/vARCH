/*
 * TempsMap.cpp
 *
 *  Created on: 19/lug/2011
 *      Author: ben
 */

#include "TempsMap.h"

#include "exceptions.h"

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
  stream << "T";
  stream.width(10);
  stream.fill('0');
  stream << uid;

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
    throw WrongArgumentException("UID not in Temporary List");
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
