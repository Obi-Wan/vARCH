/*
 * TempsMap.h
 *
 *  Created on: 19/lug/2011
 *      Author: ben
 */

#ifndef TEMPSMAP_H_
#define TEMPSMAP_H_

#include <map>
#include <set>
#include <string>

#include "macros.h"

// Used later in RegAllocator and AssemFlowGraph
typedef std::map<uint32_t, uint32_t>       AssignedRegs;

class TempsMap {
public:
  typedef class std::map<std::string, uint32_t> LabelToUID;
  typedef class std::map<uint32_t, std::string> UIDToLabel;

private:
  LabelToUID labelToUID;
  UIDToLabel uidToLabel;
public:
  void putTemp(const std::string && label, const uint32_t & uid,
      const bool & ignoreDups = false);
  void putTemp(const uint32_t & uid, const bool & ignoreDups = false);

  void clear();

  const std::string & getLabel(const uint32_t & uid) const;
  const uint32_t & getUID(const std::string & label) const;

  const LabelToUID & getLabelTable() const throw() { return labelToUID; }
  const UIDToLabel & getUIDTable() const throw() { return uidToLabel; }
};

struct AliasMap : public std::map<uint32_t, uint32_t> {
  uint32_t getFinal(const uint32_t & alias) const;

  void print() const;
};

struct ReverseAliasMap : public std::map<uint32_t, std::set<uint32_t> > {
  void add(const uint32_t & alias, const uint32_t & aliased,
      AliasMap & aliasMap);

  void print() const;
};

#endif /* TEMPSMAP_H_ */
