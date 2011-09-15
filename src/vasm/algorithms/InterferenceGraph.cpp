/*
 * InterferenceGraph.cpp
 *
 *  Created on: 01/ago/2011
 *      Author: ben
 */

#include "InterferenceGraph.h"

////////////////////////////////////////////////////////////////////////////////
/// Class AliasMap
///
/// Public Members
////////////////////////////////////////////////////////////////////////////////

void
AliasMap::add(const NodeInterfGraph<uint32_t> * const alias,
    const uint32_t & aliased)
{
  const uint32_t & uidIndex = alias->data;
  iterator index = find(uidIndex);

  if (count(uidIndex) == 0) {
    insert(value_type(uidIndex, aliased));
  } else {
    throw WrongArgumentException("Alias " + alias->label
              + " already in the aliases map");
  }
}

void
ReverseAliasMap::add(const NodeInterfGraph<uint32_t> * const alias,
    const uint32_t & aliased, AliasMap & aliasMap)
{
  iterator index = find(aliased);

  if (index == end()) {
    insert(value_type(aliased, set<uint32_t>()));
  } else {
    // update alias map
    const set<uint32_t> & aliases = index->second;
    typedef set<uint32_t>::const_iterator sa_c_iterator;

    for(sa_c_iterator al = aliases.begin(); al != aliases.end(); al++) {
      aliasMap.insert(AliasMap::value_type(*al, aliased));
    }
  }

  index->second.insert(alias->data);
  aliasMap.insert(AliasMap::value_type(alias->data, aliased));
}

////////////////////////////////////////////////////////////////////////////////
/// Class InterferenceGraph
///
/// Public Members
////////////////////////////////////////////////////////////////////////////////

InterferenceGraph::InterferenceGraph(const InterferenceGraph & other)
  : Graph<uint32_t, NodeInterfGraph>(other), moves(other.moves)
{ }

void
InterferenceGraph::printInterferenceGraph() const
{
  for(nl_c_iterator nodeIt = this->listOfNodes.begin();
      nodeIt != this->listOfNodes.end(); nodeIt++)
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
  for(nm_c_iterator mapIter = this->mapOfNodes.begin();
      mapIter != this->mapOfNodes.end(); mapIter++)
  {
    DebugPrintf((" label: %s, pointer %p\n", mapIter->first.c_str(), mapIter->second));
  }
}

bool
InterferenceGraph::hasOnlyPrecolored() const
{
  for(nl_c_iterator nodeIt = this->listOfNodes.begin();
      nodeIt != this->listOfNodes.end(); nodeIt++)
  {
    const NodeType * const node = &*nodeIt;
    if (!node->isPrecolored) { return false; }
  }
  return true;
}

