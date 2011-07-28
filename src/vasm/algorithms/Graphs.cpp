/*
 * Graphs.cpp
 *
 *  Created on: 20/lug/2011
 *      Author: ben
 */

#include "Graphs.h"

////////////////////////////////////////////////////////////////////////////////
/// Class InterferenceGraph
///
/// Public Members
////////////////////////////////////////////////////////////////////////////////

InteferenceGraph::InteferenceGraph(const InteferenceGraph & other)
  : Graph<uint32_t>(other)
{
  for(am_c_iterator moveIt = other.moves.begin(); moveIt != other.moves.end();
      moveIt++)
  {
    const string & fromLabel = moveIt->first->label;
    const NodeType * const fromNode = checkLabel(fromLabel, "");

    moves.insert(ArcsMap::value_type(fromNode, NodeSetType()));

    const NodeSetType & toSet = moveIt->second;
    for(ns_c_iterator succIt = toSet.begin(); succIt != toSet.end(); succIt++)
    {
      const NodeType * const otherToNode = *succIt;
      const string & toLabel = otherToNode->label;
      const NodeType * const toNode = checkLabel(toLabel, "");

      _addPartMoveRelation(fromNode, toNode);
      DebugPrintf(("Added move relation from %p (%s), to %p (%s)\n", fromNode,
                    fromLabel.c_str(), toNode, toLabel.c_str()));
    }
  }
}

void
InteferenceGraph::printInterferenceGraph() const
{
  for(nl_c_iterator nodeIt = this->listOfNodes.begin();
      nodeIt != this->listOfNodes.end(); nodeIt++)
  {
    const NodeType * const node = &*nodeIt;
    cout << "Node - pointer: " << node << ", label: " << node->label;
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
    am_c_iterator moveIt = moves.find(node);
    if (moveIt != moves.end()) {
      const NodeSetType & nodeMoves = moveIt->second;
      cout << "\n  Moves (size: " << nodeMoves.size() << "):" << flush;
      for(ns_c_iterator moveIt = nodeMoves.begin(); moveIt != nodeMoves.end();
          moveIt++)
      {
        cout << " " << *moveIt;
      }
    } else {
      throw WrongArgumentException("Something is missing in moves");
    }
    cout << endl;
  }
  DebugPrintf(("Map of labels\n"));
  for(nm_c_iterator mapIter = this->mapOfNodes.begin();
      mapIter != this->mapOfNodes.end(); mapIter++)
  {
    DebugPrintf((" label: %s, pointer %p\n", mapIter->first.c_str(), mapIter->second));
  }
}

