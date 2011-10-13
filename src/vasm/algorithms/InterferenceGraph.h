/*
 * InterferenceGraph.h
 *
 *  Created on: 01/ago/2011
 *      Author: ben
 */

#ifndef INTERFERENCEGRAPH_H_
#define INTERFERENCEGRAPH_H_

#include "FlowGraph.h"

#include "CpuDefinitions.h"
#include "std_istructions.h"

template<typename DataType>
class NodeInterfGraph : public NodeGraph<DataType> {
public:
  bool isPrecolored;

  bool operator==(const NodeInterfGraph<DataType> & other) const throw()
  {
    return NodeGraph<DataType>::operator ==(other)
            && (isPrecolored == other.isPrecolored);
  }
};

class InterferenceGraph : public Graph<uint32_t, NodeInterfGraph> {
public:
  typedef Graph<uint32_t, NodeInterfGraph> MovesMap;

protected:
  MovesMap moves;

public:
  InterferenceGraph();
  InterferenceGraph(const InterferenceGraph & other);

  virtual NodeType * addNewNode(const string & _label, uint32_t _data);
  virtual void removeNode(const string & _label);
  virtual void removeNode(const NodeType * const node);

  template<typename DataType>
  void populateGraph(const FlowGraph<DataType> & flowGraph,
      const LiveMap<DataType> & liveMap, const TempsMap & tempsMap);

  void printInterferenceGraph() const;

  bool hasOnlyPrecolored() const;

  const MovesMap & getMoves() const throw() { return moves; }

  bool getCoalesceCouple(const NodeType * const startNode,
      const uint32_t & maxRegs, const NodeType *& final,
      const NodeType *& alias);

  void coalesce(const NodeType * const final, const NodeType * const alias);
  void coalesce(const string & final, const string & alias);

  void fixCoalesce();

  bool nodeIsMoveRelated(const NodeType * const node);
  bool nodeHasOnlyHighDegMoves(const NodeType * const node,
      const uint32_t & limitDeg);
  bool nodeBriggsCanCoalesce(const NodeType * const node1,
      const NodeType * const node2, const uint32_t & limitDeg);
  bool nodeGeorgeCanCoalesce(const NodeType * const final,
      const NodeType * const alias, const uint32_t & limitDeg);
  void nodeFreeze(const NodeType * const node);
};


////////////////////////////////////////////////////////////////////////////////
/// Class InterferenceGraph
///
/// Private Members
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// Class InterferenceGraph
///
/// Public Members
////////////////////////////////////////////////////////////////////////////////

inline InterferenceGraph::NodeType *
InterferenceGraph::addNewNode(const string & _label, uint32_t _data)
{

  NodeType * node = Graph<uint32_t, NodeInterfGraph>::addNewNode(_label, _data);

  node->isPrecolored = (_data < (FIRST_TEMPORARY+1));

  moves.addNewNode(_label, _data);

  return node;
}

inline void
InterferenceGraph::removeNode(const string & _label)
{
  const NodeType * const node =
      checkLabel(_label, "Trying to remove a node that is not in the graph");
  moves.removeNode(_label);
  _removeNode(node);
}

inline void
InterferenceGraph::removeNode(const NodeType * const node)
{
  checkNodePtr(node, "Trying to remove a node that is not in the graph");
  moves.removeNode(node->label);
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
  moves.clear();
  this->clear();

  // Populate with nodes
  const TempsMap::LabelToUID::const_iterator endMap =
                                                 tempsMap.getLabelTable().end();
  TempsMap::LabelToUID::const_iterator itMap = tempsMap.getLabelTable().begin();
  for(; itMap != endMap; itMap++) {
    const string & tempLabel = itMap->first;
    if (!this->getMapOfNodes().count(tempLabel)) {
      this->addNewNode(tempLabel, itMap->second);
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
          throw WrongInstructionException(
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
//      DebugPrintf(("Found a move!! evaluating if it can be added to "
//                    "interference graph\n"));
      const UIDMultiSetType & nodeDefs = flowGraph.getDefs().find(node)->second;
      const UIDMultiSetType & nodeUses = flowGraph.getUses().find(node)->second;
      const string & nodeDlabel = tempsMap.getLabel(*nodeDefs.begin());
      const string & nodeUlabel = tempsMap.getLabel(*nodeUses.begin());
      const NodeType * const nodeD = checkLabel(nodeDlabel, "");
      const NodeType * const nodeU = checkLabel(nodeUlabel, "");

      DebugPrintf(("Interf: evaluating Move %s -> %s\n", nodeUlabel.c_str(),
          nodeDlabel.c_str()));

      /* Add a move only if it is not between interfering nodes!
       * And nodes are not both pre-colored */
      NodeSetType & nodeSuccs = succs.find(nodeD)->second;
      if (nodeSuccs.find(nodeU) == nodeSuccs.end()
          && !(nodeU->isPrecolored && nodeD->isPrecolored))
      {
        moves.addDirectedArc(nodeUlabel, nodeDlabel);
        DebugPrintf(("Added move relation \"%s\" -> \"%s\"\n",
            nodeUlabel.c_str(), nodeDlabel.c_str()));
      }
    }
  }
}

#endif /* INTERFERENCEGRAPH_H_ */
