/*
 * RegAllocator.h
 *
 *  Created on: 19/lug/2011
 *      Author: ben
 */

#ifndef REGALLOCATOR_H_
#define REGALLOCATOR_H_

#include "AssemFlowGraph.h"
#include "../Cpu.h"

#include <stack>

struct SimlifyRecord {
  uint32_t uid;
  bool isPotentialSpill;

  uint32_t degree;

  SimlifyRecord(const uint32_t & _uid, const uint32_t & _degree,
      const bool & _potSpill)
    : uid(_uid), isPotentialSpill(_potSpill), degree(_degree)
  { }
};

class RegAllocator {
public:
  typedef InteferenceGraph::NodeType      NodeType;
  typedef InteferenceGraph::NodeListType  NodeListType;
  typedef InteferenceGraph::NodeSetType   NodeSetType;
  typedef InteferenceGraph::ArcsMap       ArcsMap;

  typedef InteferenceGraph::nl_c_iterator nl_c_iterator;
  typedef InteferenceGraph::nl_iterator   nl_iterator;
  typedef InteferenceGraph::ns_c_iterator ns_c_iterator;
  typedef InteferenceGraph::ns_iterator   ns_iterator;
  typedef InteferenceGraph::am_c_iterator am_c_iterator;
  typedef InteferenceGraph::am_iterator   am_iterator;

  typedef map<uint32_t, uint32_t>         AssignedRegs;

protected:
  const TempsMap & tempsMap;

  deque<SimlifyRecord> nodesStack;

  // Relation 1-to-1 to nodes in the list of given interference graph
  AssignedRegs assignedRegs;

  const uint32_t maxRegs;

  void _findMax(const deque<const NodeType *> & nodes,
      const deque<uint32_t> & degrees, SimlifyRecord & record) const;
public:
  RegAllocator(const TempsMap & _temps, const uint32_t & regs = NUM_REGS)
    : tempsMap(_temps), maxRegs(regs)
  { }

  bool simpleAllocateRegs(const InteferenceGraph & interf);

  void printStack() const;
  void printAssigned() const;

  const AssignedRegs &getAssignedRegs() const throw() {return assignedRegs;}
};

#endif /* REGALLOCATOR_H_ */
