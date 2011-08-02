/*
 * TempsMap.h
 *
 *  Created on: 19/lug/2011
 *      Author: ben
 */

#ifndef TEMPSMAP_H_
#define TEMPSMAP_H_

#include <map>
#include <string>

using namespace std;

#include "macros.h"

// Used later in RegAllocator and AssemFlowGraph
typedef map<uint32_t, uint32_t>       AssignedRegs;

class TempsMap {
public:
  typedef class map<string, uint32_t> LabelToUID;
  typedef class map<uint32_t, string> UIDToLabel;

private:
  LabelToUID labelToUID;
  UIDToLabel uidToLabel;
public:
  void putTemp(const string & label, const uint32_t & uid,
      const bool & ignoreDups = false);
  void putTemp(const uint32_t & uid, const bool & ignoreDups = false);

  void clear();

  const string & getLabel(const uint32_t & uid) const;
  const uint32_t & getUID(const string & label) const;

  const LabelToUID & getLabelTable() const throw() { return labelToUID; }
  const UIDToLabel & getUIDTable() const throw() { return uidToLabel; }
};

#endif /* TEMPSMAP_H_ */
