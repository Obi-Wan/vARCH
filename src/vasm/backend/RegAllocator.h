/*
 * RegAllocator.h
 *
 *  Created on: 19/lug/2011
 *      Author: ben
 */

#ifndef REGALLOCATOR_H_
#define REGALLOCATOR_H_

#include "AssemFlowGraph.h"

#include <stack>

struct SimlifyRecord {
  typedef InteferenceGraph::NodeType      NodeType;

  const NodeType * node;
  bool isPotentialSpill;

  uint32_t degree;

  SimlifyRecord(const NodeType * const _node, const uint32_t & _degree,
      const bool & _potSpill)
    : node(_node), isPotentialSpill(_potSpill), degree(_degree)
  { }
};

class RegAllocator {
  typedef InteferenceGraph::NodeType      NodeType;
  typedef InteferenceGraph::NodeListType  NodeListType;

  typedef InteferenceGraph::nl_c_iterator nl_c_iterator;
  typedef InteferenceGraph::nl_iterator   nl_iterator;

  deque<SimlifyRecord> nodesStack;

  // Relation 1-to-1 to nodes in the list of given interference graph
  vector<uint32_t> assignedRegs;

  const uint32_t maxRegs;

  void _findMax(const deque<const NodeType *> & nodes,
      const deque<uint32_t> & degrees, SimlifyRecord & record) const;
public:
  RegAllocator(const uint32_t & regs) : maxRegs(regs) { }

  bool simpleAllocateRegs(const InteferenceGraph & interf);

  void printStack() const;

  const vector<uint32_t> &getAssignedRegs() const throw() {return assignedRegs;}
};

#endif /* REGALLOCATOR_H_ */
