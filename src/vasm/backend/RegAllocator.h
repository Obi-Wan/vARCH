/*
 * RegAllocator.h
 *
 *  Created on: 19/lug/2011
 *      Author: ben
 */

#ifndef REGALLOCATOR_H_
#define REGALLOCATOR_H_

#include "AssemFlowGraph.h"
#include "algorithms/InterferenceGraph.h"
#include "../Cpu.h"

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
  typedef InterferenceGraph::NodeType      NodeType;
  typedef InterferenceGraph::NodeListType  NodeListType;
  typedef InterferenceGraph::NodeSetType   NodeSetType;
  typedef InterferenceGraph::ArcsMap       ArcsMap;

  typedef InterferenceGraph::nl_c_iterator nl_c_iterator;
  typedef InterferenceGraph::nl_iterator   nl_iterator;
  typedef InterferenceGraph::ns_c_iterator ns_c_iterator;
  typedef InterferenceGraph::ns_iterator   ns_iterator;
  typedef InterferenceGraph::am_c_iterator am_c_iterator;
  typedef InterferenceGraph::am_iterator   am_iterator;

protected:
  const TempsMap & tempsMap;

  deque<SimlifyRecord> nodesStack;

  // Relation 1-to-1 to nodes in the list of given interference graph
  AssignedRegs assignedRegs;

  AliasMap safeAliases;
  AliasMap unsafeAliases;
  bool usingSafeAliases;

  ReverseAliasMap safeReverseAliases;
  ReverseAliasMap unsafeReverseAliases;

  const uint32_t maxRegs;

  void _findMax(const vector<const NodeType *> & nodes,
      const vector<uint32_t> & degrees, SimlifyRecord & record) const;
  void _pushNode(const SimlifyRecord & record, InterferenceGraph & graph);

  void _simpleSelect(const InterferenceGraph & interf);
  void _simpleSimplify(const InterferenceGraph & interf);
  void _coalesceSimplify(const InterferenceGraph & interf);

  void _applyCoalescingToGraph(InterferenceGraph & interf);

public:
  RegAllocator(const TempsMap & _temps, const uint32_t & regs = NUM_REGS)
    : tempsMap(_temps), usingSafeAliases(true), maxRegs(regs)
  { }

  bool simpleAllocateRegs(const InterferenceGraph & interf);
  bool coalesceAllocateRegs(const InterferenceGraph & interf);

  void printStack() const;
  void printAssigned() const;

  const AssignedRegs &getAssignedRegs() const throw() { return assignedRegs; }
  const AliasMap & getAliases() const throw() {
    return usingSafeAliases ? safeAliases : unsafeAliases;
  }
  const ReverseAliasMap & getReverseAliases() const throw() {
    return usingSafeAliases ? safeReverseAliases : unsafeReverseAliases;
  }
};

#endif /* REGALLOCATOR_H_ */
