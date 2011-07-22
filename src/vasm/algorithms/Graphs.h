/*
 * Graphs.h
 *
 *  Created on: 14/lug/2011
 *      Author: ben
 */

#ifndef GRAPHS_H_
#define GRAPHS_H_

#include <deque>
#include <set>
#include <map>
#include <string>

#include <iostream>

using namespace std;

#include "macros.h"
#include "TempsMap.h"
#include "exceptions.h"

template<typename DataType>
class NodeGraph {
public:
  DataType data;

  string label;

  bool operator==(const NodeGraph<DataType> & other) const throw()
  {
    return (label == other.label) && (data == other.data);
  }
};

template<typename DataType>
class NodeFlowGraph : public NodeGraph<DataType> {
public:
  bool isMove;

  bool operator==(const NodeFlowGraph<DataType> & other) const throw()
  {
    return NodeGraph<DataType>::operator ==(other) && (isMove == other.isMove);
  }
};


template<typename DataType, template<typename NodeDataType> class NodeBaseType = NodeGraph>
class Graph {
public:
  typedef class NodeBaseType<DataType> NodeType;
  typedef class deque<NodeType> NodeListType;
  typedef class map<const string, const NodeType * const> NodeMapType;
  typedef class set<const NodeType *> NodeSetType;
  typedef class map<const NodeType *, NodeSetType> ArcsMap;

  typedef typename NodeSetType::iterator        ns_iterator;
  typedef typename NodeSetType::const_iterator  ns_c_iterator;
  typedef typename NodeListType::iterator       nl_iterator;
  typedef typename NodeListType::const_iterator nl_c_iterator;
  typedef typename NodeMapType::iterator        nm_iterator;
  typedef typename NodeMapType::const_iterator  nm_c_iterator;
  typedef typename ArcsMap::iterator            am_iterator;

protected:
  NodeListType listOfNodes;

  ArcsMap preds;
  ArcsMap succs;

  NodeMapType mapOfNodes;

  const void * checkLabelInternal(const string & _label,
      const string & _errorMsg) const;
  NodeType * checkLabel(const string & _label, const string & _errorMsg)
    const
  { return (NodeType *) checkLabelInternal(_label, _errorMsg); }

  void checkNodePtr(const NodeType * const node, const string & errorMessage)
    const;

  void * _addNewNode(const string & _label, DataType _data);
  void _removeNode(const NodeType * const node);
  void _addDirectedArc(const NodeType * const from, const NodeType * const to);
  void _addUndirectedArc(const NodeType * const n1, const NodeType * const n2);

  size_t _inDegree(const NodeType * const node) const;
  size_t _outDegree(const NodeType * const node) const;

public:
  Graph() { }
  Graph(const Graph<DataType, NodeBaseType> & other)
    : listOfNodes(other.listOfNodes), preds(other.preds), succs(other.succs)
    , mapOfNodes(other.mapOfNodes)
  { }

  virtual void addNewNode(const string & _label, DataType _data);
  virtual void removeNode(const string & _label);
  virtual void removeNode(const NodeType * const node);

  virtual void clear();

  void addDirectedArc(const string & _from, const string & _to);
  void addDirectedArc(const NodeType * const from, const NodeType * const to);

  void addUndirectedArc(const string & node1, const string & node2);
  void addUndirectedArc(const NodeType * const n1, const NodeType * const n2);

  size_t inDegree(const NodeType * const node) const;
  size_t inDegree(const string & _label) const;
  size_t outDegree(const NodeType * const node) const;
  size_t outDegree(const string & _label) const;

  void makeVisitList(deque<const NodeType *> & list, map<const NodeType *, bool> & visited,
      const NodeType * const rootNode = NULL);

  const NodeListType & getListOfNodes() const throw() { return listOfNodes; }
  const NodeMapType & getMapOfNodes() const throw() { return mapOfNodes; }
  const ArcsMap & getPreds() const throw() { return preds; }
  const ArcsMap & getSuccs() const throw() { return succs; }
};

template<typename DataType>
struct LiveMap {
  typedef class NodeGraph<DataType> NodeType;

  typedef class set<uint32_t> UIDSetType;
  typedef class map<const NodeType *, UIDSetType> UIDsMap;

  typedef typename UIDSetType::const_iterator us_c_iterator;
  typedef typename UIDSetType::iterator       us_iterator;
  typedef typename UIDsMap::const_iterator    um_c_iterator;
  typedef typename UIDsMap::iterator          um_iterator;

  UIDsMap liveIn;
  UIDsMap liveOut;

  void printLiveMap() const;
};

template<typename DataType>
class FlowGraph : public Graph<DataType, NodeFlowGraph> {
public:
  // Inherited Internal Types
  typedef class Graph<DataType, NodeFlowGraph>::NodeType NodeType;
  typedef class Graph<DataType, NodeFlowGraph>::NodeSetType NodeSetType;
  typedef class Graph<DataType, NodeFlowGraph>::NodeListType NodeListType;
  typedef class Graph<DataType, NodeFlowGraph>::NodeMapType NodeMapType;
  typedef class Graph<DataType, NodeFlowGraph>::ArcsMap ArcsMap;

  typedef typename Graph<DataType, NodeFlowGraph>::ns_iterator    ns_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::ns_c_iterator  ns_c_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::nl_iterator    nl_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::nl_c_iterator  nl_c_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::nm_iterator    nm_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::nm_c_iterator  nm_c_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::am_iterator    am_iterator;

  // Newly Defined Types
  typedef class multiset<uint32_t> UIDMultiSetType;
  typedef class map<const NodeType *, UIDMultiSetType> UIDsMSMap;

  typedef typename UIDMultiSetType::iterator        us_iterator;
  typedef typename UIDMultiSetType::const_iterator  us_c_iterator;
  typedef typename UIDsMSMap::iterator              um_iterator;
  typedef typename UIDsMSMap::const_iterator        um_c_iterator;

protected:
  UIDsMSMap uses;
  UIDsMSMap defs;

  void _addNewUseDefRecords(const NodeType * const node);
  void _removeUseDefRecords(const NodeType * const node);

  size_t _numUses(const uint32_t & uid, const NodeType * const node) const;
  size_t _numDefs(const uint32_t & uid, const NodeType * const node) const;
  size_t _numUses(const uint32_t & uid) const;
  size_t _numDefs(const uint32_t & uid) const;

public:
  FlowGraph() { }
  FlowGraph(const FlowGraph<DataType> & other)
    : Graph<DataType, NodeFlowGraph>(other), uses(other.uses), defs(other.defs)
  { }

  virtual void addNewNode(const string & _label, DataType _data);
  virtual void removeNode(const string & _label);
  virtual void removeNode(const NodeType * const node);

  virtual void clear();

//  size_t numUses(const uint32_t & uid) const;
//  size_t numUses(const uint32_t & uid, const string & _node) const;
//  size_t numUses(const uint32_t & uid, const NodeType * const node) const;
//
//  size_t numDefs(const uint32_t & uid) const;
//  size_t numDefs(const uint32_t & uid, const string & _node) const;
//  size_t numDefs(const uint32_t & uid, const NodeType * const node) const;

  void populateLiveMap(LiveMap<DataType> & liveMap);

  const UIDsMSMap & getUses() const throw() { return uses; }
  const UIDsMSMap & getDefs() const throw() { return defs; }

  void printFlowGraph() const;
};

class InteferenceGraph : public Graph<uint32_t> {
public:
  InteferenceGraph() { }
  InteferenceGraph(const InteferenceGraph & other) : Graph<uint32_t>(other) { }

  template<typename DataType>
  void populateGraph(const FlowGraph<DataType> & flowGraph,
      const LiveMap<DataType> & liveMap, const TempsMap & tempsMap);

  void printInterferenceGraph() const;
};

////////////////////////////////////////////////////////////////////////////////
/// Class Graph
///
/// Private Members
////////////////////////////////////////////////////////////////////////////////

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline const void *
Graph<DataType, NodeBaseType>::checkLabelInternal(const string & _label,
    const string & errorMessage)
  const
{
  nm_c_iterator nodeIter = mapOfNodes.find(_label);

  if (nodeIter == mapOfNodes.end()) {
    DebugPrintf(("No label %s in map of nodes\n", _label.c_str()));
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
    DebugPrintf(("No label %s in map of nodes\n", node->label.c_str()));
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

  mapOfNodes.insert(typename NodeMapType::value_type(_label, &node));

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
  am_c_iterator predsIter = preds.find(node);
  return predsIter->second.size();
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline size_t
Graph<DataType, NodeBaseType>::_outDegree(const NodeType * const node) const
{
  am_c_iterator succsIter = succs.find(node);
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
  const NodeType * node = rootNode;
  if (listOfNodes.size()) {
    if (!node) {
      node = &listOfNodes[0];
    }

    visited.insert(typename map<const NodeType *, bool>::value_type(node, true));

    am_iterator succsIter = succs.find(node);
    if (succsIter != succs.end()) {
      const NodeSetType & succsSet = succsIter->second;
      for(ns_iterator succ = succsSet.begin(); succ != succsSet.end();
          succ++)
      {
        if (visited.find(*succ) == visited.end()) {
          this->makeVisitList(visitList, visited, *succ);
        }
      }
    } else {
      DebugPrintf(("ERROR: Could find node %p (label %s) in successive\n",
                   node, node->label.c_str()));
    }

    visitList.push_back(node);
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
  um_c_iterator live_in = liveIn.begin();
  um_c_iterator live_out = liveOut.begin();
  const um_c_iterator end_live_in = liveIn.end();
  const um_c_iterator end_live_out = liveOut.end();
  for(; live_in != end_live_in && live_out != end_live_out;
      live_in++, live_out++)
  {
    cout << "Node " << live_in->first->label << endl << "   live-in  (num: "
        << live_in->second.size() << "):";
    for(us_iterator ins = live_in->second.begin(); ins != live_in->second.end();
        ins++)
    {
      cout << " T" << (*ins +1);
    }
    cout << "\n   live-out (num: " << live_out->second.size() << "):";
    for(us_iterator outs = live_out->second.begin();
        outs != live_out->second.end(); outs++)
    {
      cout << " T" << (*outs +1);
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
  uses.insert(typename UIDsMSMap::value_type(node, UIDMultiSetType() ));
  defs.insert(typename UIDsMSMap::value_type(node, UIDMultiSetType() ));
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
  typedef typename deque<const NodeType *>::iterator        nd_iterator;
  typedef typename deque<const NodeType *>::const_iterator  nd_c_iterator;

  typedef class set<uint32_t> UIDSetType;
  typedef class map<const NodeType *, UIDSetType> UIDsMap;

  // Reset live map
  liveMap.liveIn.clear();
  liveMap.liveOut.clear();

  for(nl_iterator listIter = this->listOfNodes.begin();
      listIter != this->listOfNodes.end(); listIter++)
  {
    const NodeType * const node = &*listIter;
    const UIDMultiSetType & nodeUses = uses[node];
    liveMap.liveIn.insert(
        typename UIDsMap::value_type(node, UIDSetType(nodeUses.begin(),
                                                             nodeUses.end())) );
    liveMap.liveOut.insert(
        typename UIDsMap::value_type(node, UIDSetType()) );
  }

  deque<const NodeType *> visitList;
  map<const NodeType *, bool> visited;

  this->makeVisitList(visitList, visited);

  uint32_t iter = 0;
  for(bool modified = true; modified;)
  {
    modified = false;

    DebugPrintf(("Liveness: iteration %u. VisitList Size: %lu\n", iter++,
        visitList.size()));

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
        const NodeSetType & nodePreds = this->preds.find(node)->second;
        const ns_iterator endPreds = nodePreds.end();
        for(ns_iterator predIt = nodePreds.begin(); predIt != endPreds;
            predIt++)
        {
          const NodeType * const pred = *predIt;
          UIDSetType & outPred = liveMap.liveOut[pred];

          if (outPred.find(*live_in) == outPred.end()) {
            DebugPrintf(( "Adding Live-Out: T%5d to node %s\n",
                          *live_in, pred->label.c_str()));
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
  for(nl_c_iterator nodeIt = this->listOfNodes.begin();
      nodeIt != this->listOfNodes.end(); nodeIt++)
  {
    const NodeType * const node = &*nodeIt;
    cout << "Node - pointer: " << node << "\n  label: " << node->label
        << "\n  isMove: " << boolalpha << node->isMove;
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
    cout << "\n  Uses:";
    const UIDMultiSetType & nodeUses = uses.find(node)->second;
    for(us_c_iterator useIt = nodeUses.begin(); useIt != nodeUses.end();
        useIt++)
    {
      cout << " T" << (*useIt +1);
    }
    cout << "\n  Defs:";
    const UIDMultiSetType & nodeDefs = defs.find(node)->second;
    for(us_c_iterator defIt = nodeDefs.begin(); defIt != nodeDefs.end();
        defIt++)
    {
      cout << " T" << (*defIt +1);
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

        if (*live_out != *nodeUses.begin()) {
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
}

#endif /* GRAPHS_H_ */
