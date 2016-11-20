/*
 * RegAllocator.cpp
 *
 *  Created on: 19/lug/2011
 *      Author: ben
 */

#include "RegAllocator.h"

inline void
RegAllocator::_findMax(const std::vector<const NodeType *> & nodes,
    const std::vector<uint32_t> & degrees, SimlifyRecord & record)
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

inline void
RegAllocator::_pushNode(const SimlifyRecord & record, InterferenceGraph & graph)
{
  nodesStack.push_back(record);

  const std::string &tempLbl = tempsMap.getLabel(record.uid);
  DebugPrintf(("Selected temp: %u\n  label: %s\n  degree: %u\n", record.uid,
      tempLbl.c_str(), record.degree));

  graph.removeNode(tempLbl);
}

inline void
RegAllocator::_simpleSimplify(const InterferenceGraph & interf)
{
  InterferenceGraph workGraph(interf);
  for(; !workGraph.hasOnlyPrecolored();)
  {
#ifdef DEBUG
    DebugPrintf(("Iteration of simplify. Graph Size: %lu\n",
        workGraph.getListOfNodes().size()));
    workGraph.printInterferenceGraph();
    DebugPrintf(("Printed partial InterfGraph\n"));
#endif

    std::vector<const NodeType *> significantNodes, notSignificantNodes;
    std::vector<uint32_t> significantDegrees, notSignificantDegrees;

    for(nl_c_iterator nodeIt = workGraph.getListOfNodes().begin();
        nodeIt != workGraph.getListOfNodes().end(); nodeIt++)
    {
      const NodeType * const node = &*nodeIt;
      if (!node->isPrecolored) {
        const uint32_t degree = workGraph.outDegree(node);
        if (degree < maxRegs) {
          notSignificantNodes.push_back(node);
          notSignificantDegrees.push_back(degree);
        } else {
          significantNodes.push_back(node);
          significantDegrees.push_back(degree);
        }
      }
    }

    SimlifyRecord record(0, 0, notSignificantNodes.empty());
    if (record.isPotentialSpill) {
      _findMax(significantNodes, significantDegrees, record);
    } else {
      _findMax(notSignificantNodes, notSignificantDegrees, record);
    }

    _pushNode(record, workGraph);
  }
#ifdef DEBUG
  // Debug Section
  DebugPrintf(("End of iterations for simplify. Graph Size: %lu\n",
      workGraph.getListOfNodes().size()));
  workGraph.printInterferenceGraph();
  DebugPrintf(("Printed final InterfGraph (with just precolored nodes)\n"));

  printStack();
  // End Debug section
#endif
}

inline void
RegAllocator::_coalesceSimplify(const InterferenceGraph & interf)
{
  usingSafeAliases = true;

  InterferenceGraph workGraph(interf);
  for(; !workGraph.hasOnlyPrecolored();)
  {
    std::vector<const NodeType *> highDegNodes, simplifyNodes, freezeNodes;
    std::vector<uint32_t> highDegNodesDegs, simplifyNodesDegs, freezeNodesDegs;

#ifdef DEBUG
    DebugPrintf(("Iteration of simplify. Graph Size: %lu\n",
        workGraph.getListOfNodes().size()));
    workGraph.printInterferenceGraph();
    DebugPrintf(("Printed partial InterfGraph\n"));
#endif

    for(nl_c_iterator nodeIt = workGraph.getListOfNodes().begin();
        nodeIt != workGraph.getListOfNodes().end(); nodeIt++)
    {
      const NodeType * const node = &*nodeIt;
      if (!node->isPrecolored) {
        const uint32_t degree = workGraph.outDegree(node);
        if (degree < maxRegs) {
          if ( workGraph.nodeIsMoveRelated(node) )
          {
            DebugPrintf(("Node %u is move related\n", node->data));
            freezeNodes.push_back(node);
            freezeNodesDegs.push_back(degree);
          } else {
            DebugPrintf(("Node %u is NOT move related\n", node->data));
            simplifyNodes.push_back(node);
            simplifyNodesDegs.push_back(degree);
          }
        } else {
          highDegNodes.push_back(node);
          highDegNodesDegs.push_back(degree);
        }
      }
    }

    DebugPrintf(("Trying Simplify\n"));
    /* Simplify Phase - Testing if possible */
    if (!simplifyNodes.empty()) {
      SimlifyRecord record(0, 0, false);
      _findMax(simplifyNodes, simplifyNodesDegs, record);
      DebugPrintf(("Simplifying node: %u\n", record.uid));
      _pushNode(record, workGraph);

      /* Simplify was successful so let's go to the next iteration */
      continue;
    }

    DebugPrintf(("Trying Coalesce\n"));
    /* Coalesce, Freeze or Potential Spill Phase */

    /* First of all, let's find a node in the freeze list that can be
     * coalesced */
    bool coalesced = false;

    for(size_t numFreeze = 0; numFreeze < freezeNodes.size() && !coalesced;
        numFreeze++)
    {
      const NodeType * final = NULL, * alias = NULL;
      const NodeType * const startNode = freezeNodes[numFreeze];

      if (workGraph.getCoalesceCouple(startNode, this->maxRegs, final, alias))
      {
        DebugPrintf(("Coalescing nodes %u <- %u\n", final->data, alias->data));

        if (final == alias) throw WrongArgumentException("Same final & alias");

        if (usingSafeAliases) {
          safeReverseAliases.add(alias->data, final->data, safeAliases);
        } else {
          unsafeReverseAliases.add(alias->data, final->data, unsafeAliases);
        }

        workGraph.coalesce(final, alias);
        workGraph.fixCoalesce();

        coalesced = true;
      }
    }

    if (coalesced) {
      /* We coalesced, so let's go to the next iteration */
      continue;
    }

    DebugPrintf(("Trying Freeze\n"));
    if(!freezeNodes.empty()) {
      DebugPrintf(("Freezing node: %u\n", freezeNodes[0]->data));
      /* Let's try to freeze a move related node! */
      workGraph.nodeFreeze(freezeNodes[0]);

      /* We finally gave up coalescing that node */
      continue;
    }

    DebugPrintf(("Doing Potential Spill\n"));
    /* Freeze not possible, let's do potential spill! */
    SimlifyRecord record(0, 0, true);
    _findMax(highDegNodes, highDegNodesDegs, record);
    _pushNode(record, workGraph);

    usingSafeAliases = false;

    unsafeAliases.insert(safeAliases.begin(), safeAliases.end());
    unsafeReverseAliases.insert(safeReverseAliases.begin(),
                                safeReverseAliases.end());
  }

#ifdef DEBUG
  // Debug Section
  DebugPrintf(("End of iterations for simplify. Graph Size: %lu\n",
      workGraph.getListOfNodes().size()));
  workGraph.printInterferenceGraph();
  DebugPrintf(("Printed final InterfGraph (with just precolored nodes)\n"));

  printStack();
  // End Debug section
#endif
}


inline void
RegAllocator::_applyCoalescingToGraph(InterferenceGraph & interf)
{
  DebugPrintf(("--> Emit final coalesced graph <--\n"));
  for(AliasMap::const_iterator alIt = getAliases().begin();
      alIt != getAliases().end(); alIt++)
  {
    interf.coalesce(
        tempsMap.getLabel(alIt->second), tempsMap.getLabel(alIt->first));

    /* Useless to do fixCoalescing here, since we are just interested in
     * interferences. If it become useful, just uncomment */
//    interf.fixCoalesce();
  }
  DebugPrintf(("--> Emitted final coalesced graph <--\n\n"));
}

inline void
RegAllocator::_simpleSelect(const InterferenceGraph & interf)
{
  // Registers run from 1 to 8, 0 is for Actual Spill
  std::vector<bool> regs;
  for(; nodesStack.size(); )
  {
    regs.clear(); regs.resize(maxRegs, true);
    SimlifyRecord record = nodesStack.back(); nodesStack.pop_back();

    const std::string &tempLbl = tempsMap.getLabel(record.uid);
    am_c_iterator succs = interf.getSuccs().find(interf.checkLabel(tempLbl,""));

    const NodeSetType & rels = succs->second;
    DebugPrintf(("Node %s\n", tempLbl.c_str()));
    for(ns_c_iterator relIt = rels.begin(); relIt != rels.end(); relIt++)
    {
      const NodeType * const rel = *relIt;
      DebugPrintf(("Interferes with %p, %s\n", rel, rel->label.c_str()));
      AssignedRegs::iterator prevAssigned = assignedRegs.find(rel->data);
      if (prevAssigned != assignedRegs.end()) {
        DebugPrintf(("  With assigned: %u\n", prevAssigned->second -1));
        regs[prevAssigned->second -1] = false;
      } else if (rel->isPrecolored) {
        DebugPrintf(("  With pre-assigned: %u\n", rel->data -1));
        regs[rel->data -1] = false;
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
}

bool
RegAllocator::simpleAllocateRegs(const InterferenceGraph & interf)
{
  _simpleSimplify(interf);

  _simpleSelect(interf);

#ifdef DEBUG
  printAssigned();
#endif

  // Return true if there were no spills
  for(AssignedRegs::iterator assIt = assignedRegs.begin();
      assIt != assignedRegs.end(); assIt++)
  {
    if (assIt->second == 0) return false;
  }
  return true;
}

bool
RegAllocator::coalesceAllocateRegs(const InterferenceGraph & interf)
{
  // Resetting also safe ones, since it does not provide spilling
  safeAliases.clear(); safeReverseAliases.clear();
  unsafeAliases.clear(); unsafeReverseAliases.clear();

  _coalesceSimplify(interf);

  InterferenceGraph coalescedGraph(interf);
  _applyCoalescingToGraph(coalescedGraph);

  _simpleSelect(coalescedGraph);

#ifdef DEBUG
  printAssigned();
#endif

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
  std::cout << "Stack of simplified nodes (number of registers " << maxRegs << "):"
      << std::endl;
  for(std::deque<SimlifyRecord>::const_reverse_iterator el = nodesStack.rbegin();
      el != nodesStack.rend(); ++el)
  {
    std::cout << "Node: " << el->uid << "\n  label: " << tempsMap.getLabel(el->uid)
        << "\n  degree: " << el->degree
        << "\n  Potential Spill: " << std::boolalpha << el->isPotentialSpill << std::endl;
  }
}

void
RegAllocator::printAssigned() const
{
  std::cout << "Assigned regs to temps: \n";
  for(AssignedRegs::const_iterator el = assignedRegs.begin();
      el != assignedRegs.end(); el++)
  {
    std::cout << "Node: " << tempsMap.getLabel(el->first) << ", reg: ";
    if (el->second) {
      std::cout << "R" << el->second << std::endl;
    } else {
      std::cout << "Actual Spill" << std::endl;
    }
  }
}
