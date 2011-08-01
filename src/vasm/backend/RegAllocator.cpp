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
    const uint32_t & uid = nodes[nodeNum]->data;
    const uint32_t & degree = degrees[nodeNum];
    if (degree >= record.degree) {
      record.uid = uid;
      record.degree = degree;
    }
  }
}

bool
RegAllocator::simpleAllocateRegs(const InterferenceGraph & interf)
{
  InterferenceGraph workGraph(interf);
  for(; workGraph.getListOfNodes().size();)
  {
    DebugPrintf(("Iteration of simplify. Graph Size: %lu\n",
        workGraph.getListOfNodes().size()));
    workGraph.printInterferenceGraph();
    DebugPrintf(("Printed partial InterfGraph\n"));

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

    SimlifyRecord record(0, 0, notSignificantNodes.empty());
    if (record.isPotentialSpill) {
      _findMax(significantNodes, significantDegrees, record);
    } else {
      _findMax(notSignificantNodes, notSignificantDegrees, record);
    }
    nodesStack.push_back(record);

    const string &tempLbl = tempsMap.getLabel(record.uid);
    DebugPrintf(("Selected temp: %u\n  label: %s\n  degree: %u\n", record.uid,
        tempLbl.c_str(), record.degree));

    workGraph.removeNode(tempLbl);
  }
//  DebugPrintf(("Iteration of simplify. Graph Size: %lu\n",
//      workGraph.getListOfNodes().size()));
//  workGraph.printInterferenceGraph();
//  DebugPrintf(("Printed partial InterfGraph\n"));

  printStack();

  // Registers run from 1 to 8, 0 is for Actual Spill
  vector<bool> regs;
  for(; nodesStack.size(); )
  {
    regs.clear(); regs.resize(maxRegs, true);
    SimlifyRecord record = nodesStack.back(); nodesStack.pop_back();

    const string &tempLbl = tempsMap.getLabel(record.uid);
    am_c_iterator succs = interf.getSuccs().find(interf.checkLabel(tempLbl,""));

    const NodeSetType & rels = succs->second;
    DebugPrintf(("Node %s\n", tempLbl.c_str()));
    for(ns_c_iterator relIt = rels.begin(); relIt != rels.end(); relIt++)
    {
      DebugPrintf(("Interferisce con %p, %s\n", *relIt, (*relIt)->label.c_str()));
      AssignedRegs::iterator prevAssigned = assignedRegs.find((*relIt)->data);
      if (prevAssigned != assignedRegs.end()) {
        DebugPrintf(("  Con assegnato: %u\n", prevAssigned->second -1));
        regs[prevAssigned->second -1] = false;
      }
    }
    uint32_t assigned = 0;
    for(uint32_t reg = 0; reg < maxRegs; reg++)
    {
      if (regs[reg]) {
        assigned = reg+1;
        break;
      }
    }
    assignedRegs.insert(AssignedRegs::value_type(record.uid, assigned));
  }

  printAssigned();

  // Return true if there were no spills
  for(AssignedRegs::iterator assIt = assignedRegs.begin();
      assIt != assignedRegs.end(); assIt++)
  {
    if (assIt->second == 0) return false;
  }
  return true;
}

void
RegAllocator::printStack() const
{
  cout << "Stack of simplified nodes (number of registers " << maxRegs << "):"
      << endl;
  for(deque<SimlifyRecord>::const_reverse_iterator el = nodesStack.rbegin();
      el != nodesStack.rend(); ++el)
  {
    cout << "Node: " << el->uid << "\n  label: " << tempsMap.getLabel(el->uid)
        << "\n  degree: " << el->degree
        << "\n  Potential Spill: " << boolalpha << el->isPotentialSpill << endl;
  }
}

void
RegAllocator::printAssigned() const
{
  cout << "Assigned regs to temps: \n";
  for(AssignedRegs::const_iterator el = assignedRegs.begin();
      el != assignedRegs.end(); el++)
  {
    cout << "Node: " << tempsMap.getLabel(el->first) << ", reg: ";
    if (el->second) {
      cout << "R" << el->second << endl;
    } else {
      cout << "Actual Spill" << endl;
    }
  }
}
