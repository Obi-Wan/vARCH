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

using namespace std;

#include "macros.h"
#include "TempsMap.h"

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
protected:
  typedef class NodeBaseType<DataType> NodeType;
  typedef class set<const NodeType *> NodeSetType;
  typedef class deque<NodeType> NodeListType;
  typedef class map<string, const NodeType *> NodeMapType;
  typedef class map<const NodeType *, NodeSetType> ArcsMap;

  typedef typename NodeSetType::iterator ns_iterator;
  typedef typename NodeListType::iterator nl_iterator;
  typedef typename NodeMapType::iterator nm_iterator;
  typedef typename ArcsMap::iterator am_iterator;

  NodeListType listOfNodes;

  ArcsMap preds;
  ArcsMap succs;

  NodeMapType mapOfNodes;

  void * checkLabelInternal(const string & _label,
      const string & _errorMsg) const;
  NodeType * checkLabel(const string & _label, const string & _errorMsg)
    const
  { return (NodeType *) checkLabel(_label, _errorMsg); }

  void checkNodePtr(const NodeType * const node, const string & errorMessage)
    const;

  void * _addNewNode(const string & _label, DataType _data);
  void _removeNode(const NodeType * const node);
  void _addDirectedArc(const NodeType * const from, const NodeType * const to);
  void _addUndirectedArc(const NodeType * const n1, const NodeType * const n2);

  size_t _inDegree(const NodeType * const node) const;
  size_t _outDegree(const NodeType * const node) const;

public:
//  Graph();

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

  typedef typename UIDSetType::iterator us_iterator;
  typedef typename UIDsMap::iterator um_iterator;

  UIDsMap liveIn;
  UIDsMap liveOut;

  void printLiveMap() const;
};

template<typename DataType>
class FlowGraph : public Graph<DataType, NodeFlowGraph> {
protected:
  // Inherited Internal Types
  typedef class Graph<DataType, NodeFlowGraph>::NodeType NodeType;
  typedef class Graph<DataType, NodeFlowGraph>::NodeSetType NodeSetType;
  typedef class Graph<DataType, NodeFlowGraph>::NodeListType NodeListType;
  typedef class Graph<DataType, NodeFlowGraph>::NodeMapType NodeMapType;
  typedef class Graph<DataType, NodeFlowGraph>::ArcsMap ArcsMap;

  typedef typename Graph<DataType, NodeFlowGraph>::ns_iterator ns_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::nl_iterator nl_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::nm_iterator nm_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::am_iterator am_iterator;

  // Newly Defined Types
  typedef class multiset<uint32_t> UIDSetType;
  typedef class map<const NodeType *, UIDSetType> UIDsMap;

  typedef typename UIDSetType::iterator us_iterator;
  typedef typename UIDsMap::iterator um_iterator;

  UIDsMap uses;
  UIDsMap defs;

  void _addNewUseDefRecords(const NodeType * const node);
  void _removeUseDefRecords(const NodeType * const node);

  size_t _numUses(const uint32_t & uid, const NodeType * const node) const;
  size_t _numDefs(const uint32_t & uid, const NodeType * const node) const;
  size_t _numUses(const uint32_t & uid) const;
  size_t _numDefs(const uint32_t & uid) const;

public:
//  FlowGraph();

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

  const UIDsMap & getUses() const throw() { return uses; }
  const UIDsMap & getDefs() const throw() { return defs; }

  void printFlowGraph() const;
};

class InteferenceGraph : public Graph<uint32_t> {
protected:
  typedef class multiset<uint32_t> UIDSetType;
  typedef class map<NodeType *, UIDSetType> UIDsMap;
public:
//  InteferenceGraph();

  template<typename DataType>
  void populateGraph(const FlowGraph<DataType> & flowGraph,
      const LiveMap<DataType> & liveMap, const TempsMap & tempsMap);
};


#endif /* GRAPHS_H_ */
