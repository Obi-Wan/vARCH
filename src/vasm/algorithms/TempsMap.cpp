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

////////////////////////////////////////////////////////////////////////////////
/// Class AliasMap
///
/// Public Members
////////////////////////////////////////////////////////////////////////////////

uint32_t
AliasMap::getFinal(const uint32_t & alias) const
{
  const_iterator finalIt = find(alias);
  if (finalIt != end()) {
    return finalIt->second;
  } else {
    return 0;
  }
}

void
AliasMap::print() const
{
  cout << "Printing Aliases Map:" << endl;
  for(const_iterator it = begin(); it != end(); it++)
  {
    cout << "  - " << it->second << " <- " << it->first << endl;
  }
  cout << "Printed Aliases Map" << endl << endl;
}

void
ReverseAliasMap::add(const uint32_t & alias, const uint32_t & final,
    AliasMap & aliasMap)
{
  typedef set<uint32_t>::const_iterator sa_c_iterator;

  iterator finalIt = find(final);

  /* If destination is not in ReverseMap yet, let's create an entry */
  if (finalIt == end()) {
    finalIt = insert(value_type(final, set<uint32_t>())).first;
  }
  /* Let's add the reverse relation */
  set<uint32_t> & finalAliases = finalIt->second;
  finalAliases.insert(alias);

  /* Let's see if alias was a 'final', and in case move all the aliases */
  iterator aliasIt = find(alias);
  if (aliasIt != end()) {
    const set<uint32_t> & aliases = aliasIt->second;

    /* Copy the aliases to the new final (In the Alias Map) */
    for(sa_c_iterator al = aliases.begin(); al != aliases.end(); al++) {
      aliasMap[*al] = final;
    }

    /* Copy the aliases to the new final (In the Reverse Alias Map) */
    finalAliases.insert(aliases.begin(), aliases.end());

    /* Erase the alias from the finals in Reverse Alias Map */
    erase(aliasIt);
  }
  /* Let's add the relation to the alias map */
  aliasMap[alias] = final;
}

void
ReverseAliasMap::print() const
{
  cout << "--> Printing Reverse Aliases Map: <--" << endl;
  for(const_iterator it = begin(); it != end(); it++)
  {
    cout << "  - " << it->first << " -> \"";
    const set<uint32_t> & aliases = it->second;
    for(set<uint32_t>::const_iterator itAlias = aliases.begin();
        itAlias != aliases.end(); itAlias++)
    {
      cout << " " << *itAlias;
    }
    cout << " \"" << endl;
  }
  cout << "--> Printed Reverse Aliases Map <--" << endl << endl;
}
