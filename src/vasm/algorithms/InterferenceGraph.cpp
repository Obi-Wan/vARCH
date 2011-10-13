/*
 * InterferenceGraph.cpp
 *
 *  Created on: 01/ago/2011
 *      Author: ben
 */

#include "InterferenceGraph.h"

////////////////////////////////////////////////////////////////////////////////
/// Class InterferenceGraph
///
/// Public Members
////////////////////////////////////////////////////////////////////////////////

InterferenceGraph::InterferenceGraph()
  : Graph<uint32_t, NodeInterfGraph>(), moves(this->baseGraph, false)
{ }

InterferenceGraph::InterferenceGraph(const InterferenceGraph & other)
  : Graph<uint32_t, NodeInterfGraph>(other), moves(this->baseGraph, false)
{
  DebugPrintf(("Copy constructor of InterferenceGraph\n"));
  for(am_c_iterator predIt = other.moves.getPreds().begin();
      predIt != other.moves.getPreds().end();
      predIt++)
  {
    const string & fromLabel = predIt->first->label;
    const NodeSetType & toSet = predIt->second;
    for(ns_c_iterator succIt = toSet.begin(); succIt != toSet.end(); succIt++)
    {
      const NodeType * const toNode = *succIt;
      moves.addDirectedArc(fromLabel, toNode->label);
      DebugPrintf(("Added MOVE arc: from %s (%p), to %s (%p)\n",
          fromLabel.c_str(), checkLabel(fromLabel,""),
          toNode->label.c_str(), checkLabel(toNode->label,"")));
    }
  }
}

void
InterferenceGraph::printInterferenceGraph() const
{
  for(nl_c_iterator nodeIt = getListOfNodes().begin();
      nodeIt != getListOfNodes().end(); nodeIt++)
  {
    const NodeType * const node = &*nodeIt;
    cout << "Node - pointer: " << node << ", label: " << node->label
          << ", pre-colored: " << boolalpha << node->isPrecolored;

    CHECK_THROW((this->preds.count(node) != 0),
        WrongArgumentException("Node: " + node->label + " not in preds"));
    CHECK_THROW((this->succs.count(node) != 0),
        WrongArgumentException("Node: " + node->label + " not in succs"));

    cout << "\n  Preds:";
    const NodeSetType & nodePreds = this->preds.find(node)->second;
    for(ns_c_iterator predIt = nodePreds.begin(); predIt != nodePreds.end();
        predIt++)
    {
      cout << " " << *predIt;
    }
    cout << "\n  Succs:";
    const NodeSetType & nodeSuccs = this->succs.find(node)->second;
    for(ns_c_iterator succIt = nodeSuccs.begin(); succIt != nodeSuccs.end();
        succIt++)
    {
      cout << " " << *succIt;
    }
    {
      const MovesMap::NodeType * const moveNode
                                            = moves.checkLabel(node->label, "");

      cout << "\n  Incoming Moves:";
      const MovesMap::NodeSetType & inMoves
                                      = moves.getPreds().find(moveNode)->second;
      for(MovesMap::ns_c_iterator inIt = inMoves.begin(); inIt != inMoves.end();
          inIt++)
      {
        cout << " " << *inIt;
      }
      cout << "\n  Outgoing Moves:";
      const MovesMap::NodeSetType & outMoves
                                      = moves.getSuccs().find(moveNode)->second;
      for(MovesMap::ns_c_iterator outIt = outMoves.begin();
          outIt != outMoves.end(); outIt++)
      {
        cout << " " << *outIt;
      }
    }
    cout << endl;
  }
  DebugPrintf(("Map of labels\n"));
  for(nm_c_iterator mapIter = getMapOfNodes().begin();
      mapIter != getMapOfNodes().end(); mapIter++)
  {
    DebugPrintf((" label: %s, pointer %p\n", mapIter->first.c_str(), mapIter->second));
  }
}

bool
InterferenceGraph::hasOnlyPrecolored() const
{
  for(nl_c_iterator nodeIt = getListOfNodes().begin();
      nodeIt != getListOfNodes().end(); nodeIt++)
  {
    const NodeType * const node = &*nodeIt;
    if (!node->isPrecolored) { return false; }
  }
  return true;
}

bool
InterferenceGraph::nodeIsMoveRelated(const NodeType * const node)
{
  return (moves.inDegree(node->label) || moves.outDegree(node->label));
}

bool
InterferenceGraph::nodeHasOnlyHighDegMoves(const NodeType * const node,
    const uint32_t & limitDeg)
{
  // It implements George's Coalescing criterion.
  const MovesMap::NodeSetType & outMoves = moves.getSuccs(node);
  const MovesMap::NodeSetType & inMoves = moves.getPreds(node);

  for(MovesMap::NodeSetType::const_iterator out_m = outMoves.begin();
      out_m != outMoves.end(); out_m++)
  {
    const NodeType * const nodeAdj_m = (*out_m);
    if (inDegree(nodeAdj_m) < limitDeg || areAdjacent(node, nodeAdj_m)) {
      return false;
    }
  }

  for(MovesMap::NodeSetType::const_iterator in_m = inMoves.begin();
      in_m != inMoves.end(); in_m++)
  {
    const NodeType * const nodeAdj_m = (*in_m);
    if (inDegree(nodeAdj_m) < limitDeg || areAdjacent(node, nodeAdj_m)) {
      return false;
    }
  }

  return true;
}

bool
InterferenceGraph::nodeBriggsCanCoalesce(const NodeType * const node1,
    const NodeType * const node2, const uint32_t & limitDeg)
{
  DebugPrintf(("Briggs criterion for %u <- %u\n", node1->data, node2->data));
  NodeSetType adjNodes;
  uint32_t numHighDeg = 0;

  const NodeSetType & incoming1 = preds.find(node1)->second;
  adjNodes.insert(incoming1.begin(), incoming1.end());

  const NodeSetType & incoming2 = preds.find(node2)->second;
  adjNodes.insert(incoming2.begin(), incoming2.end());

  for(ns_iterator it = adjNodes.begin(); it != adjNodes.begin(); it++)
  {
    const NodeType * const node = *it;
    if (inDegree(node) >= limitDeg) numHighDeg++;
  }

  return (numHighDeg < limitDeg);
}

bool
InterferenceGraph::nodeGeorgeCanCoalesce(const NodeType * const final,
    const NodeType * const alias, const uint32_t & limitDeg)
{
  bool canCoalesce = true;
  const NodeSetType & adjNodesAlias = getPreds(alias);

  DebugPrintf(("George criterion for %u <- %u\n", final->data, alias->data));
  for(ns_iterator it = adjNodesAlias.begin();
      it != adjNodesAlias.begin() && canCoalesce; it++)
  {
    const NodeType * const adj = *it;
    DebugPrintf(("Considering Adjacent %u\n", adj->data));
    canCoalesce = canCoalesce
        && (inDegree(adj) < limitDeg || adj->isPrecolored
            || areAdjacent(final, adj));
  }

  return canCoalesce;
}

void
InterferenceGraph::nodeFreeze(const NodeType * const node)
{
  moves.removeAllArcs(node->label);
}

void
InterferenceGraph::coalesce(const NodeType * const final,
    const NodeType * const alias)
{
  moves.coalesce(final, alias);
  this->_coalesce(final, alias);
}

void
InterferenceGraph::coalesce(const string & final, const string & alias)
{

  moves.coalesce(final, alias);
  Graph<uint32_t, NodeInterfGraph>::coalesce(final, alias);
}

void
InterferenceGraph::fixCoalesce()
{
  for(am_c_iterator moveIt = moves.getSuccs().begin();
      moveIt != moves.getSuccs().end(); moveIt++)
  {
    const NodeType * const node = moveIt->first;

    /* Set of moves from the given node: Explicit override of const behavior */
    NodeSetType & movePointed = (NodeSetType &) moveIt->second;

    /* If the move connects the same node */
    movePointed.erase(node);

    /* Set of interferences of the given node */
    const NodeSetType & nodeInterf = succs.find(node)->second;

    /* If the move connects points that interfere, then delete the move arc */
    for(ns_c_iterator interfIt = nodeInterf.begin();
        interfIt != nodeInterf.end(); interfIt++)
    {
      const NodeType * const nodePointed = (*interfIt);
      if (movePointed.count(nodePointed)) {
        moves.delDirectedArc(node, nodePointed);
      }
    }

    /* If the move connects two precolored points, then delete the move arc */
    if (node->isPrecolored) {
      for(ns_c_iterator moveAdjIt = movePointed.begin();
          moveAdjIt != movePointed.end(); moveAdjIt++)
      {
        const NodeType * const nodePointed = (*moveAdjIt);
        if (nodePointed->isPrecolored) {
          moves.delDirectedArc(node, nodePointed);
        }
      }
    }
  }
  for(am_c_iterator moveIt = moves.getPreds().begin();
      moveIt != moves.getPreds().end(); moveIt++)
  {
    const NodeType * const node = moveIt->first;

    /* Set of moves from the given node: Explicit override of const behavior */
    NodeSetType & movePointed = (NodeSetType &) moveIt->second;

    /* If the move connects the same node */
    movePointed.erase(node);
  }
}

bool
InterferenceGraph::getCoalesceCouple(const NodeType * const startNode,
    const uint32_t & maxRegs, const NodeType *& final, const NodeType *& alias)
{
  NodeSetType moveAdjSet;

  const NodeSetType & incoming1 = moves.getPreds(startNode);
  moveAdjSet.insert(incoming1.begin(), incoming1.end());

  const NodeSetType & incoming2 = moves.getSuccs(startNode);
  moveAdjSet.insert(incoming2.begin(), incoming2.end());

  for(ns_iterator adjIt = moveAdjSet.begin(); adjIt != moveAdjSet.end();
      adjIt++)
  {
    final = startNode->isPrecolored ? startNode : *adjIt;
    alias = startNode->isPrecolored ? *adjIt : startNode;

    if (final->isPrecolored) {
      /* For precolored Nodes we use George criterion */
      if (nodeGeorgeCanCoalesce(final, alias, maxRegs)) return true;
    } else {
      /* For ordinary Nodes we use Briggs criterion */
      if (nodeBriggsCanCoalesce(final, alias, maxRegs)) return true;
    }
  }
  return false;
}


