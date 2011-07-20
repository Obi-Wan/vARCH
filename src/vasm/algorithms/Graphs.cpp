/*
 * Graphs.cpp
 *
 *  Created on: 14/lug/2011
 *      Author: ben
 */

#ifndef GRAPHS_CPP_
#define GRAPHS_CPP_

#include "Graphs.h"

#include "exceptions.h"

#include <iostream>

////////////////////////////////////////////////////////////////////////////////
/// Class Graph
///
/// Private Members
////////////////////////////////////////////////////////////////////////////////

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void *
Graph<DataType, NodeBaseType>::checkLabelInternal(const string & _label,
    const string & errorMessage)
  const
{
  nm_iterator nodeIter = mapOfNodes.find(_label);

  if (nodeIter == mapOfNodes.end()) {
    throw WrongArgumentException(errorMessage);
  }

  return nodeIter->second;
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void
Graph<DataType, NodeBaseType>::checkNodePtr(const NodeType * const node,
    const string & errorMessage)
  const
{
  if (!node) { throw WrongArgumentException("Null pointer as node pointer"); }
  if (mapOfNodes.find(node->label) == mapOfNodes.end()) {
    throw WrongArgumentException(errorMessage);
  }
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void *
Graph<DataType, NodeBaseType>::_addNewNode(const string & _label,
    DataType _data)
{
  listOfNodes.push_back(NodeType());
  NodeType & node = listOfNodes.back();

  node.data = _data;
  node.label = _label;

  preds.insert(typename ArcsMap::value_type(&node, NodeSetType() ));
  succs.insert(typename ArcsMap::value_type(&node, NodeSetType() ));

  return &node;
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void
Graph<DataType, NodeBaseType>::_removeNode(const NodeType * const node)
{
  {
    am_iterator predsIter = preds.find(node);
    if (predsIter != preds.end()) {
      NodeSetType & nodePreds = predsIter->second;

      for(ns_iterator listIter = nodePreds.begin(); listIter != nodePreds.end();
          listIter++)
      {
        am_iterator succIter = succs.find(*listIter);
        if (succIter != succs.end()) {
          succIter->second.erase(node);
        }
      }
    }
    preds.erase(predsIter);
  }
  {
    am_iterator succsIter = succs.find(node);
    if (succsIter != succs.end()) {
      NodeSetType & nodeSuccs = succsIter->second;

      for(ns_iterator listIter = nodeSuccs.begin(); listIter != nodeSuccs.end();
          listIter++)
      {
        am_iterator predIter = preds.find(*listIter);
        if (predIter != preds.end()) {
          predIter->second.erase(node);
        }
      }
    }
    succs.erase(succsIter);
  }
  mapOfNodes.erase(node->label);

  for(nl_iterator listIter = listOfNodes.begin(); listIter != listOfNodes.end();
      listIter++)
  {
    if (*listIter == *node) {
      listOfNodes.erase(listIter);
      break;
    }
  }
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void
Graph<DataType, NodeBaseType>::_addDirectedArc(const NodeType * const from,
    const NodeType * const to)
{
  {
    am_iterator toIter = preds.find(to);
    if (toIter != preds.end()) {
      NodeSetType & toPreds = toIter->second;
      toPreds.insert(from);
    }
  }
  {
    am_iterator fromIter = succs.find(from);
    if (fromIter != succs.end()) {
      NodeSetType & fromSuccs = fromIter->second;
      fromSuccs.insert(to);
    }
  }
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void
Graph<DataType, NodeBaseType>::_addUndirectedArc(const NodeType * const n1,
    const NodeType * const n2)
{
  _addDirectedArc(n1,n2);
  _addDirectedArc(n2,n1);
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline size_t
Graph<DataType, NodeBaseType>::_inDegree(const NodeType * const node) const
{
  am_iterator predsIter = preds.find(node);
  return predsIter->second.size();
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline size_t
Graph<DataType, NodeBaseType>::_outDegree(const NodeType * const node) const
{
  am_iterator succsIter = succs.find(node);
  return succsIter->second.size();
}

////////////////////////////////////////////////////////////////////////////////
/// Class Graph
///
/// Public Members
////////////////////////////////////////////////////////////////////////////////

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
void
Graph<DataType, NodeBaseType>::addNewNode(const string & _label,
    DataType _data)
{
  if (this->mapOfNodes.count(_label)) {
    throw DuplicateLabelException("Label: '" + _label +
        "' already associated to a node in the graph");
  }

  _addNewNode(_label, _data);
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
void
Graph<DataType, NodeBaseType>::removeNode(const NodeType * const node)
{
  checkNodePtr(node, "Trying to remove a node that is not in the graph");
  _removeNode(node);
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
void
Graph<DataType, NodeBaseType>::removeNode(const string & _label)
{
  _removeNode(
      checkLabel(_label, "Trying to remove a node that is not in the graph"));
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
void
Graph<DataType, NodeBaseType>::clear()
{
  mapOfNodes.clear();
  preds.clear();
  succs.clear();
  listOfNodes.clear();
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
void
Graph<DataType, NodeBaseType>::addDirectedArc(const string & _from,
    const string & _to)
{
  _addDirectedArc(
      checkLabel(_from, "Trying to add an arc from a node not in the graph"),
      checkLabel(_to, "Trying to add an arc to a node not in the graph")
      );
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
void
Graph<DataType, NodeBaseType>::addDirectedArc(const NodeType * const from,
    const NodeType * const to)
{
  checkNodePtr(from, "Trying to add an arc from a node not in the graph");
  checkNodePtr(to, "Trying to add an arc to a node not in the graph");

  _addDirectedArc(from, to);
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
void
Graph<DataType, NodeBaseType>::addUndirectedArc(const string & node1,
    const string & node2)
{
  const string errorMsg = "Trying to add an arc from (and to) a node not in the"
      " graph";

  _addUndirectedArc(
      checkLabel(node1, errorMsg),
      checkLabel(node2, errorMsg)
      );
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
void
Graph<DataType, NodeBaseType>::addUndirectedArc(const NodeType * const node1,
    const NodeType * const node2)
{
  const string errorMsg = "Trying to add an arc from (and to) a node not in the"
      " graph";

  checkNodePtr(node1, errorMsg);
  checkNodePtr(node2, errorMsg);

  _addUndirectedArc(node1, node2);
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
size_t
Graph<DataType, NodeBaseType>::inDegree(const NodeType * const node) const
{
  checkNodePtr(node, "Trying to get the In Degree of a node not in the graph");
  return _inDegree(node);
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
size_t
Graph<DataType, NodeBaseType>::inDegree(const string & _label) const
{
  return _inDegree(
      checkLabel( _label,
                  "Trying to get the In Degree of a node not in the graph")
      );
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
size_t
Graph<DataType, NodeBaseType>::outDegree(const NodeType * const node) const
{
  checkNodePtr(node, "Trying to get the Out Degree of a node not in the graph");
  return _outDegree(node);
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
size_t
Graph<DataType, NodeBaseType>::outDegree(const string & _label) const
{
  return _inDegree(
      checkLabel( _label,
                  "Trying to get the Out Degree of a node not in the graph")
      );
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
void
Graph<DataType, NodeBaseType>::makeVisitList(
    deque<const NodeType *> & visitList, map<const NodeType *, bool> & visited,
    const NodeType * const rootNode)
{
  if (listOfNodes.size()) {
    if (!rootNode) {
      rootNode = listOfNodes[0];
    }

    visited.insert(map<NodeType *, bool>::value_type(rootNode, true));

    am_iterator succsIter = succs.find(rootNode);
    if (succsIter != succs.end()) {
      const NodeSetType & succsSet = succsIter->second;
      for(ns_iterator succ = succsSet.begin(); succ != succsSet.end();
          succ++)
      {
        if (visited.find(*succ) != visited.end()) {
          this->makeVisitList(visitList, visited, *succ);
        }
      }
    }

    visitList.push_back(rootNode);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Class LiveMap
///
/// Public Members
////////////////////////////////////////////////////////////////////////////////

template<typename DataType>
void
LiveMap<DataType>::printLiveMap() const
{
  um_iterator live_in = liveIn.begin();
  um_iterator live_out = liveOut.begin();
  const um_iterator end_live_in = liveIn.end();
  const um_iterator end_live_out = liveOut.end();
  for(; live_in != end_live_in && live_out != end_live_out;
      live_in++, live_out++)
  {
    cout << "Node " << live_in->first.label << ", live-in:";
    for(us_iterator ins = live_in->second.begin(); ins != live_in->second.end();
        ins++)
    {
      cout << " " << *ins;
    }
    cout << ", live-out:";
    for(us_iterator outs = live_out->second.begin();
        outs != live_out->second.end(); outs++)
    {
      cout << " " << *outs;
    }
    cout << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Class FlowGraph
///
/// Private Members
////////////////////////////////////////////////////////////////////////////////

template<typename DataType>
void
FlowGraph<DataType>::_addNewUseDefRecords(const NodeType * const node)
{
  uses.insert(typename UIDsMap::value_type(node, UIDSetType() ));
  defs.insert(typename UIDsMap::value_type(node, UIDSetType() ));
}

template<typename DataType>
void
FlowGraph<DataType>::_removeUseDefRecords(const NodeType * const node)
{
  uses.erase(node);
  defs.erase(node);
}

template<typename DataType>
size_t
FlowGraph<DataType>::_numUses(const uint32_t & uid, const NodeType * const node)
  const
{
  const um_iterator usesIter = uses.find(node);
  return usesIter->second.count(uid);
}

template<typename DataType>
size_t
FlowGraph<DataType>::_numDefs(const uint32_t & uid, const NodeType * const node)
  const
{
  const um_iterator defsIter = defs.find(node);
  return defsIter->second.count(uid);
}

template<typename DataType>
size_t
FlowGraph<DataType>::_numUses(const uint32_t & uid) const
{
  size_t tempCount = 0;
  for(um_iterator usesIter = uses.begin(); usesIter != uses.end(); usesIter++)
  {
    tempCount += usesIter->second.count(uid);
  }
  return tempCount;
}

template<typename DataType>
size_t
FlowGraph<DataType>::_numDefs(const uint32_t & uid) const
{
  size_t tempCount = 0;
  for(um_iterator defsIter = defs.begin(); defsIter != defs.end(); defsIter++)
  {
    tempCount += defsIter->second.count(uid);
  }
  return tempCount;
}

////////////////////////////////////////////////////////////////////////////////
/// Class FlowGraph
///
/// Public Members
////////////////////////////////////////////////////////////////////////////////

template<typename DataType>
void
FlowGraph<DataType>::addNewNode(const string & _label, DataType _data)
{
  if (this->mapOfNodes.find(_label) != this->mapOfNodes.end()) {
    throw DuplicateLabelException("Label: '" + _label +
        "' already associated to a node in the graph");
  }

  NodeType * const node = (NodeType *) this->_addNewNode(_label, _data);
  _addNewUseDefRecords(node);
}

template<typename DataType>
void
FlowGraph<DataType>::removeNode(const NodeType * const node)
{
  this->checkNodePtr(node, "Trying to remove a node that is not in the graph");

  _removeUseDefRecords(node);
  this->_removeNode(node);
}

template<typename DataType>
void
FlowGraph<DataType>::removeNode(const string & _label)
{
  NodeType * const node =
      this->checkLabel( _label,
                        "Trying to remove a node that is not in the graph");

  _removeUseDefRecords(node);
  this->_removeNode(node);
}

template<typename DataType>
void
FlowGraph<DataType>::clear()
{
  Graph<DataType, NodeFlowGraph>::clear();
  uses.clear();
  defs.clear();
}

template<typename DataType>
void
FlowGraph<DataType>::populateLiveMap(LiveMap<DataType> & liveMap)
{
  typedef typename deque<NodeType *>::iterator nd_iterator;

  // Reset live map
  liveMap.liveIn.clear();
  liveMap.liveOut.clear();

  for(nl_iterator listIter = this->listOfNodes.begin();
      listIter != this->listOfNodes.end(); listIter++)
  {
    const NodeType * const node = &*listIter;
    const UIDSetType & nodeUses = uses[node];
    liveMap.liveIn.insert(node, UIDSetType(nodeUses.begin(), nodeUses.end()));
    liveMap.liveOut.insert(node, UIDSetType());
  }

  deque<NodeType *> visitList;
  map<NodeType *, bool> visited;

  this->makeVisitList(visitList, visited);

  bool modified = true;

  while(modified) {
    modified = false;

    for(nd_iterator listIter = visitList.begin(); listIter != visitList.end();
        listIter++)
    {
      const NodeType * const node = *listIter;

      UIDSetType & nodeLiveIn = liveMap.liveIn[node];
      UIDSetType & nodeLiveOut = liveMap.liveOut[node];

      const us_iterator endLivesIn = nodeLiveIn.end();
      for(us_iterator live_in = nodeLiveIn.begin(); live_in != endLivesIn;
          live_in++)
      {
        const ns_iterator endPreds = this->preds[node].end();
        for(ns_iterator pred = this->preds[node].begin(); pred != endPreds;
            pred++)
        {
          UIDSetType & outPred = liveMap.liveOut[*pred];
          if (outPred.find(*live_in) == outPred.end()) {
            outPred.insert(*live_in);
            modified = true;
          }
        }
      }

      const us_iterator endLivesOut = nodeLiveOut.end();
      for(us_iterator live_out = nodeLiveOut.begin(); live_out != endLivesOut;
          live_out++)
      {
        if ( (nodeLiveIn.find(*live_out) == nodeLiveIn.end())
            && (defs[node].find(*live_out) == defs[node].end()))
        {
          nodeLiveIn.insert(*live_out);
          modified = true;
        }
      }
    }
  }
}

template<typename DataType>
void
FlowGraph<DataType>::printFlowGraph() const
{
  for(nl_iterator nodeIt = this->listOfNodes.begin();
      nodeIt != this->listOfNodes.end(); nodeIt++)
  {
    const NodeType * const node = *nodeIt;
    cout << "Node: " << node->label << ", isMove: " << node->isMove;
    cout << "\n  Preds:";
    cout << "\n  Succs:";
    cout << "\n  Uses:";
    for(us_iterator useIt = uses[node].begin(); useIt != uses[node].end();
        useIt++)
    {
      cout << " " << *useIt;
    }
    cout << "\n  Defs:";
    for(us_iterator defIt = defs[node].begin(); defIt != defs[node].end();
        defIt++)
    {
      cout << " " << *defIt;
    }
    cout << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Class InterferenceGraph
///
/// Public Members
////////////////////////////////////////////////////////////////////////////////

template<typename DataType>
void
InteferenceGraph::populateGraph(const FlowGraph<DataType> & flowGraph,
    const LiveMap<DataType> & liveMap, const TempsMap & tempsMap)
{
  typedef class FlowGraph<DataType>::NodeType NodeType;
  typedef class FlowGraph<DataType>::NodeSetType NodeSetType;
  typedef class FlowGraph<DataType>::NodeListType NodeListType;
  typedef class FlowGraph<DataType>::NodeMapType NodeMapType;
  typedef class FlowGraph<DataType>::ArcsMap ArcsMap;

  typedef typename FlowGraph<DataType>::ns_iterator ns_iterator;
  typedef typename FlowGraph<DataType>::nl_iterator nl_iterator;
  typedef typename FlowGraph<DataType>::nm_iterator nm_iterator;
  typedef typename FlowGraph<DataType>::am_iterator am_iterator;

  typedef typename UIDSetType::iterator us_iterator;
  typedef typename UIDsMap::iterator um_iterator;

  this->clear();

  for(nl_iterator nodeIt = flowGraph.getListOfNodes().begin();
      nodeIt != flowGraph.getListOfNodes().end(); nodeIt++)
  {
    const NodeType * const node = &*nodeIt;

    UIDSetType & nodeLiveIn = liveMap.liveIn[node];
    UIDSetType & nodeLiveOut = liveMap.liveOut[node];

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
  for(nl_iterator nodeIt = flowGraph.getListOfNodes().begin();
      nodeIt != flowGraph.getListOfNodes().end(); nodeIt++)
  {
    const NodeType * const node = &*nodeIt;
    UIDSetType & nodeLiveOut = liveMap.liveOut[node];

    const us_iterator endLivesOut = nodeLiveOut.end();
    for(us_iterator live_out = nodeLiveOut.begin(); live_out != endLivesOut;
        live_out++)
    {
      UIDSetType & nodeDefs = flowGraph.getDefs()[node];
      if (node->isMove) {
        // it should have both one define and one use
        UIDSetType & nodeUses = flowGraph.getUses()[node];
        // Safety check
        if (!(nodeDefs.size() == 1 && nodeUses.size() == 1)) {
          throw WrongIstructionException(
              "A move instruction should have both one define and one use");
        }
        if (*live_out != *nodeUses.begin()) {
          addUndirectedArc(*nodeDefs.begin(), *live_out);
        }
      } else {
        for(us_iterator defsIter = nodeDefs.begin(); defsIter != nodeDefs.end();
            defsIter++)
        {
          addUndirectedArc(*defsIter, *live_out);
        }
      }
    }
  }
}

#endif /* GRAPHS_CPP_ */
