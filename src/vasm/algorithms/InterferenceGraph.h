/*
 * InterferenceGraph.h
 *
 *  Created on: 01/ago/2011
 *      Author: ben
 */

#ifndef INTERFERENCEGRAPH_H_
#define INTERFERENCEGRAPH_H_

#include "FlowGraph.h"

class InterferenceGraph : public Graph<uint32_t> {
  ArcsMap moves;

  void _addPartMoveRelation(const NodeType * const node1,
      const NodeType * const node2);
  void _removeMoves(const NodeType * const node);
public:
  InterferenceGraph() { }
  InterferenceGraph(const InterferenceGraph & other);

  virtual void addNewNode(const string & _label, uint32_t _data);
  virtual void removeNode(const string & _label);
  virtual void removeNode(const NodeType * const node);

  template<typename DataType>
  void populateGraph(const FlowGraph<DataType> & flowGraph,
      const LiveMap<DataType> & liveMap, const TempsMap & tempsMap);

  void printInterferenceGraph() const;
};


////////////////////////////////////////////////////////////////////////////////
/// Class InterferenceGraph
///
/// Private Members
////////////////////////////////////////////////////////////////////////////////

inline void
InterferenceGraph::_addPartMoveRelation(const NodeType * const node1,
    const NodeType * const node2)
{
  am_iterator moves1 = moves.find(node1);
  if (moves1 != moves.end()) {
    moves1->second.insert(node2);
  } else {
    throw WrongArgumentException("Not existing node '" + node1->label
        + "' in moves map");
  }
}

inline void
InterferenceGraph::_removeMoves(const NodeType * const node)
{
  am_iterator nodeMoves = moves.find(node);
  if (nodeMoves != moves.end()) {
    NodeSetType & nodeSetMoves = nodeMoves->second;
    for(ns_iterator moveIt = nodeSetMoves.begin(); moveIt != nodeSetMoves.end();
        moveIt++)
    {
      am_iterator nodeOtherMoves = moves.find(*moveIt);
      if (nodeOtherMoves != moves.end()) {
        NodeSetType & nodeSetOtherMoves = nodeOtherMoves->second;
        ns_iterator backRefMove = nodeSetOtherMoves.find(node);
        if (backRefMove != nodeSetOtherMoves.end()) {
          nodeSetOtherMoves.erase(backRefMove);
        }
      } else {
        throw WrongArgumentException("Not existing destination for node '"
            + node->label + "' in moves map");
      }
    }
    moves.erase(nodeMoves);
  } else {
    throw WrongArgumentException("Not existing node '" + node->label
        + "' in moves map");
  }
  DebugPrintf(("Removed moves of node %p\n", node));
}

////////////////////////////////////////////////////////////////////////////////
/// Class InterferenceGraph
///
/// Public Members
////////////////////////////////////////////////////////////////////////////////

inline void
InterferenceGraph::addNewNode(const string & _label, uint32_t _data)
{
  Graph<uint32_t>::addNewNode(_label, _data);
  const NodeType * const node = &listOfNodes.back();

  moves.insert(ArcsMap::value_type(node, NodeSetType()));
}

inline void
InterferenceGraph::removeNode(const string & _label)
{
  const NodeType * const node =
      checkLabel(_label, "Trying to remove a node that is not in the graph");
  _removeMoves(node);
  _removeNode(node);
}

inline void
InterferenceGraph::removeNode(const NodeType * const node)
{
  checkNodePtr(node, "Trying to remove a node that is not in the graph");
  _removeMoves(node);
  _removeNode(node);
}

template<typename DataType>
void
InterferenceGraph::populateGraph(const FlowGraph<DataType> & flowGraph,
    const LiveMap<DataType> & liveMap, const TempsMap & tempsMap)
{
  typedef class FlowGraph<DataType>::NodeType     FG_NodeType;
//  typedef class FlowGraph<DataType>::NodeSetType  NodeSetType;
//  typedef class FlowGraph<DataType>::NodeListType NodeListType;
//  typedef class FlowGraph<DataType>::NodeMapType  NodeMapType;
//  typedef class FlowGraph<DataType>::ArcsMap      ArcsMap;

//  typedef typename FlowGraph<DataType>::ns_iterator fg_ns_iterator;
//  typedef typename FlowGraph<DataType>::nl_iterator fg_nl_iterator;
//  typedef typename FlowGraph<DataType>::nm_iterator fg_nm_iterator;
//  typedef typename FlowGraph<DataType>::am_iterator fg_am_iterator;

//  typedef typename FlowGraph<DataType>::ns_c_iterator fg_ns_c_iterator;
  typedef typename FlowGraph<DataType>::nl_c_iterator fg_nl_c_iterator;
//  typedef typename FlowGraph<DataType>::nm_c_iterator fg_nm_c_iterator;
//  typedef typename FlowGraph<DataType>::am_c_iterator fg_am_c_iterator;

  typedef class FlowGraph<DataType>::UIDMultiSetType UIDMultiSetType;

  typedef class LiveMap<DataType>::UIDSetType UIDSetType;
  typedef class LiveMap<DataType>::UIDsMap    UIDsMap;

  typedef typename UIDSetType::iterator us_iterator;
  typedef typename UIDsMap::iterator um_iterator;

  // Clean graph
  this->clear();

  // Populate with nodes
  const fg_nl_c_iterator endOfNodes = flowGraph.getListOfNodes().end();
  for(fg_nl_c_iterator nodeIt = flowGraph.getListOfNodes().begin();
      nodeIt != endOfNodes; nodeIt++)
  {
    const FG_NodeType * const node = &*nodeIt;

    const UIDSetType & nodeLiveIn = liveMap.liveIn.find(node)->second;
    const UIDSetType & nodeLiveOut = liveMap.liveOut.find(node)->second;

    const us_iterator endLivesIn = nodeLiveIn.end();
    for(us_iterator live_in = nodeLiveIn.begin(); live_in != endLivesIn;
        live_in++)
    {
      const string & tempLabel = tempsMap.getLabel(*live_in);
      if (!this->mapOfNodes.count(tempLabel)) {
        this->addNewNode(tempLabel, *live_in);
      }
    }

    const us_iterator endLivesOut = nodeLiveOut.end();
    for(us_iterator live_out = nodeLiveOut.begin(); live_out != endLivesOut;
        live_out++)
    {
      const string & tempLabel = tempsMap.getLabel(*live_out);
      if (!this->mapOfNodes.count(tempLabel)) {
        this->addNewNode(tempLabel, *live_out);
      }
    }
  }

  // Find interference relations
  for(fg_nl_c_iterator nodeIt = flowGraph.getListOfNodes().begin();
      nodeIt != flowGraph.getListOfNodes().end(); nodeIt++)
  {
    const FG_NodeType * const node = &*nodeIt;
    const UIDSetType & nodeLiveOut = liveMap.liveOut.find(node)->second;

    const us_iterator endLivesOut = nodeLiveOut.end();
    for(us_iterator live_out = nodeLiveOut.begin(); live_out != endLivesOut;
        live_out++)
    {
      const UIDMultiSetType & nodeDefs = flowGraph.getDefs().find(node)->second;
      if (node->isMove) {
        // it should have both one define and one use
        const UIDMultiSetType & nodeUses =
                                         flowGraph.getUses().find(node)->second;
        // Safety check - Could be skipped
        if (!(nodeDefs.size() == 1 && nodeUses.size() == 1)) {
          throw WrongIstructionException(
              "A move instruction should have both one define and one use");
        }

        if (*live_out != *nodeUses.begin() && *live_out != *nodeDefs.begin()) {
          addUndirectedArc( tempsMap.getLabel(*nodeDefs.begin()),
                            tempsMap.getLabel(*live_out) );
        }
      } else {
        for(us_iterator defsIter = nodeDefs.begin(); defsIter != nodeDefs.end();
            defsIter++)
        {
          if (*defsIter != *live_out) {
            addUndirectedArc( tempsMap.getLabel(*defsIter),
                              tempsMap.getLabel(*live_out) );
          }
        }
      }
    }
  }

  // Sub-optimal - Find move relations
  for(fg_nl_c_iterator nodeIt = flowGraph.getListOfNodes().begin();
      nodeIt != flowGraph.getListOfNodes().end(); nodeIt++)
  {
    const FG_NodeType * const node = &*nodeIt;
    if (node->isMove) {
      const UIDMultiSetType & nodeDefs = flowGraph.getDefs().find(node)->second;
      const UIDMultiSetType & nodeUses = flowGraph.getUses().find(node)->second;
      const string & nodeDlabel = tempsMap.getLabel(*nodeDefs.begin());
      const string & nodeUlabel = tempsMap.getLabel(*nodeUses.begin());
      const NodeType * const nodeD = checkLabel(nodeDlabel, "");
      const NodeType * const nodeU = checkLabel(nodeUlabel, "");

      NodeSetType & nodeSuccs = succs.find(nodeD)->second;
      if (nodeSuccs.find(nodeU) == nodeSuccs.end()) {
        _addPartMoveRelation(nodeD, nodeU);
        _addPartMoveRelation(nodeU, nodeD);
      }
    }
  }
}

#endif /* INTERFERENCEGRAPH_H_ */
