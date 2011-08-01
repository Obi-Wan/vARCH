/*
 * Graphs.h
 *
 *  Created on: 14/lug/2011
 *      Author: ben
 */

#ifndef GRAPHS_H_
#define GRAPHS_H_

#include <list>
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
  typedef class list<NodeType> NodeListType;
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
  typedef typename ArcsMap::const_iterator      am_c_iterator;

protected:
  NodeListType listOfNodes;

  ArcsMap preds;
  ArcsMap succs;

  NodeMapType mapOfNodes;

  const void * checkLabelInternal(const string & _label,
      const string & _errorMsg) const;

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
  Graph(const Graph<DataType, NodeBaseType> & other);

  virtual void addNewNode(const string & _label, DataType _data);
  virtual void removeNode(const string & _label);
  virtual void removeNode(const NodeType * const node);

  virtual void clear();

  NodeType * checkLabel(const string & _label, const string & _errorMsg)
    const
  { return (NodeType *) checkLabelInternal(_label, _errorMsg); }

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
Graph<DataType, NodeBaseType>::Graph(const Graph<DataType, NodeBaseType> & old)
  : listOfNodes(old.listOfNodes)
{
  for(nl_iterator nodeIt = listOfNodes.begin(); nodeIt != listOfNodes.end();
      nodeIt++)
  {
    const NodeType * const node = &*nodeIt;
    mapOfNodes.insert(typename NodeMapType::value_type(node->label,node));
    DebugPrintf(("Inserted pair: %p, %s\n", node, node->label.c_str()));

    preds.insert(typename ArcsMap::value_type(node, NodeSetType() ));
    succs.insert(typename ArcsMap::value_type(node, NodeSetType() ));
  }
  for(am_c_iterator predIt = old.preds.begin(); predIt != old.preds.end();
      predIt++)
  {
    const string & fromLabel = predIt->first->label;
    const NodeSetType & toSet = predIt->second;
    for(ns_c_iterator succIt = toSet.begin(); succIt != toSet.end(); succIt++)
    {
      const NodeType * const toNode = *succIt;
      this->addDirectedArc(fromLabel, toNode->label);
      DebugPrintf(("Added arc: from %s (%p), to %s (%p)\n",
          fromLabel.c_str(), checkLabel(fromLabel,""),
          toNode->label.c_str(), checkLabel(toNode->label,"")));
    }
  }
}

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
      node = &*listOfNodes.begin();
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

#endif /* GRAPHS_H_ */
