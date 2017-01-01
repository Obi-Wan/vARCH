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

////////////////////////////////////////////////////////////////////////////////
/// Class TempsMap
///
/// Public Members
////////////////////////////////////////////////////////////////////////////////

void
TempsMap::putTemp(const std::string && label, const uint32_t & uid,
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
  std::stringstream stream;
  stream.fill('0');
  switch (uid) {
    case 1 ... NUM_REGS: {
      stream << "R";
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
    case FRAME_POINTER+1: {
      stream << "FP";
      break;
    }
    case STATE_REGISTER+1: {
      stream << "SR";
      break;
    }
    case PROGRAM_COUNTER+1: {
      stream << "PC";
      break;
    }
    default: {
      stream << "T";
      stream.width(10);
      stream << uid;
      break;
    }
  }
  putTemp(move(stream.str()), uid, ignoreDups);
}

void
TempsMap::clear()
{
  uidToLabel.clear();
  labelToUID.clear();
}

const std::string &
TempsMap::getLabel(const uint32_t & uid) const
{
  UIDToLabel::const_iterator label = uidToLabel.find(uid);
  if (label == uidToLabel.end()) {
    std::stringstream stream;
    stream << "UID: " << uid << " not in Temporary List";
    throw WrongArgumentException(stream.str());
  }
  return label->second;
}

const uint32_t &
TempsMap::getUID(const std::string & label) const
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
  const_iterator finalIt = this->find(alias);
  if (finalIt != end()) {
    return finalIt->second;
  } else {
    return 0;
  }
}

void
AliasMap::print() const
{
  std::cout << "Printing Aliases Map:" << std::endl;
  for(const_iterator it = this->begin(); it != this->end(); it++)
  {
    std::cout << "  - " << it->second << " <- " << it->first << std::endl;
  }
  std::cout << "Printed Aliases Map" << std::endl << std::endl;
}

void
ReverseAliasMap::add(const uint32_t & alias, const uint32_t & final,
    AliasMap & aliasMap)
{
  typedef std::set<uint32_t>::const_iterator sa_c_iterator;

  iterator finalIt = this->find(final);

  /* If destination is not in ReverseMap yet, let's create an entry */
  if (finalIt == this->end()) {
    finalIt = insert(value_type(final, std::set<uint32_t>())).first;
  }
  /* Let's add the reverse relation */
  std::set<uint32_t> & finalAliases = finalIt->second;
  finalAliases.insert(alias);

  /* Let's see if alias was a 'final', and in case move all the aliases */
  iterator aliasIt = find(alias);
  if (aliasIt != end()) {
    const std::set<uint32_t> & aliases = aliasIt->second;

    /* Copy the aliases to the new final (In the Alias Map) */
    for(const uint32_t & al : aliases)
    {
      aliasMap[al] = final;
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
  std::cout << "--> Printing Reverse Aliases Map: <--" << std::endl;
  for(const_iterator it = this->begin(); it != this->end(); it++)
  {
    std::cout << "  - " << it->first << " -> \"";

    for(const uint32_t & alias : it->second)
    {
      std::cout << " " << alias;
    }
    std::cout << " \"" << std::endl;
  }
  std::cout << "--> Printed Reverse Aliases Map <--" << std::endl << std::endl;
}
