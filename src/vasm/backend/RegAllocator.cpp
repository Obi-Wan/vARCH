/*
 * RegAllocator.cpp
 *
 *  Created on: 19/lug/2011
 *      Author: ben
 */

#include "RegAllocator.h"

void
RegAllocator::_findMax(const deque<const NodeType *> & nodes,
    const deque<uint32_t> & degrees, SimlifyRecord & record)
  const
{
  for(size_t nodeNum = 0; nodeNum < nodes.size(); nodeNum++)
  {
    const NodeType * const node = nodes[nodeNum];
    const uint32_t & degree = degrees[nodeNum];
    if (degree > record.degree) {
      record.node = node;
      record.degree = degree;
    }
  }
}

bool
RegAllocator::simpleAllocateRegs(const InteferenceGraph & interf)
{
  for(InteferenceGraph workGraph(interf); workGraph.getListOfNodes().size();)
  {
    DebugPrintf(("Iteration of simplify. Graph Size: %lu\n",
        workGraph.getListOfNodes().size()));
    deque<const NodeType *> significantNodes, notSignificantNodes;
    deque<uint32_t> significantDegrees, notSignificantDegrees;

    for(nl_c_iterator nodeIt = workGraph.getListOfNodes().begin();
        nodeIt != workGraph.getListOfNodes().end(); nodeIt++)
    {
      const NodeType * const node = &*nodeIt;
      const uint32_t degree = workGraph.outDegree(node);
      if (degree < maxRegs) {
        notSignificantNodes.push_back(node);
        notSignificantDegrees.push_back(degree);
      } else {
        significantNodes.push_back(node);
        significantDegrees.push_back(degree);
      }
    }

    SimlifyRecord record(NULL, 0, notSignificantNodes.empty());
    if (record.isPotentialSpill) {
      _findMax(significantNodes, significantDegrees, record);
    } else {
      _findMax(notSignificantNodes, notSignificantDegrees, record);
    }
    nodesStack.push_back(record);

    DebugPrintf(("Selected node: %p\n  label: %s\n  degree: %u\n", record.node,
        record.node->label.c_str(), record.degree));

    workGraph.removeNode(record.node);
  }

  printStack();
}

void
RegAllocator::printStack() const
{
  cout << "Stack of simplified nodes (number of registers " << maxRegs << "):"
      << endl;
  for(deque<SimlifyRecord>::const_reverse_iterator el = nodesStack.rend();
      el != nodesStack.rbegin(); el--)
  {
    cout << "Node: " << el->node << "\n  label: " << el->node->label
        << "\n  degree: " << el->degree
        << "\n  Potential Spill: " << boolalpha << el->isPotentialSpill << endl;
  }
}
