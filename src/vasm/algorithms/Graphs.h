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

#include <iostream>

#include "BaseGraph.h"
#include "TempsMap.h"


template<typename DataType, template<typename NodeDataType> class NodeBaseType = NodeGraph>
class Graph {
public:
  typedef typename BaseGraph<DataType, NodeBaseType>::NodeType     NodeType;
  typedef typename BaseGraph<DataType, NodeBaseType>::NodeListType NodeListType;
  typedef typename BaseGraph<DataType, NodeBaseType>::NodeMapType  NodeMapType;
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
  BaseGraph<DataType, NodeBaseType> * baseGraph;
  const bool baseGraphIsPrivate;

  ArcsMap preds;
  ArcsMap succs;

  void _removeNode(const NodeType * const node);
  void _delDirectedArc(const NodeType * const from, const NodeType * const to);
  void _addDirectedArc(const NodeType * const from, const NodeType * const to);
  void _delUndirectedArc(const NodeType * const n1, const NodeType * const n2);
  void _addUndirectedArc(const NodeType * const n1, const NodeType * const n2);
  void _removeAllArcs(const NodeType * const node, const bool &noTrace = false);

  size_t _inDegree(const NodeType * const node) const;
  size_t _outDegree(const NodeType * const node) const;

public:
  Graph()
    : baseGraph(new BaseGraph<DataType, NodeBaseType>())
    , baseGraphIsPrivate(true)
  { }
  Graph(BaseGraph<DataType, NodeBaseType> * bg, const bool & copy = true);
  Graph(const Graph<DataType, NodeBaseType> & other);

  virtual ~Graph() { if (baseGraph && baseGraphIsPrivate) delete baseGraph; }

  virtual NodeType * addNewNode(const string & _label, DataType _data);
  virtual void removeNode(const string & _label);
  virtual void removeNode(const NodeType * const node);

  virtual void clear();

  void addDirectedArc(const string & _from, const string & _to);
  void addDirectedArc(const NodeType * const from, const NodeType * const to);

  void addUndirectedArc(const string & node1, const string & node2);
  void addUndirectedArc(const NodeType * const n1, const NodeType * const n2);

  void removeAllArcs(const string & node1);
  void removeAllArcs(const NodeType * const n1);

  size_t inDegree(const NodeType * const node) const;
  size_t inDegree(const string & _label) const;
  size_t outDegree(const NodeType * const node) const;
  size_t outDegree(const string & _label) const;

  void makeVisitList(deque<const NodeType *> & list,
      map<const NodeType *, bool> & visited,
      const NodeType * const rootNode = NULL);

  const ArcsMap & getPreds() const throw() { return preds; }
  const ArcsMap & getSuccs() const throw() { return succs; }

  const NodeSetType & getPreds(const NodeType * const node) const;
  const NodeSetType & getSuccs(const NodeType * const node) const;

  bool areAdjacent(const NodeType * const n1, const NodeType * const n2) const;

  /* Wrappers to BaseGraph */

  void checkNodePtr(const NodeType * const node, const string & errorMessage)
    const
  { baseGraph->checkNodePtr(node, errorMessage); }

  NodeType * checkLabel(const string & _label, const string & _errorMsg)
  { return baseGraph->checkLabel(_label, _errorMsg); }
  const NodeType * checkLabel(const string & _label, const string & _errorMsg)
      const
  { return baseGraph->checkLabel(_label, _errorMsg); }

  const NodeListType & getListOfNodes() const throw()
  { return baseGraph->getListOfNodes(); }
  const NodeMapType & getMapOfNodes() const throw()
  { return baseGraph->getMapOfNodes(); }
};

////////////////////////////////////////////////////////////////////////////////
/// Class Graph
///
/// Private Members
////////////////////////////////////////////////////////////////////////////////

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void
Graph<DataType, NodeBaseType>::_removeNode(const NodeType * const node)
{
  _removeAllArcs(node, true);
  baseGraph->_removeNode(node);
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void
Graph<DataType, NodeBaseType>::_removeAllArcs(const NodeType * const node,
    const bool & noTrace)
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
      if (noTrace) {
        preds.erase(predsIter);
      } else {
        nodePreds.clear();
      }
    }
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
      if (noTrace) {
        succs.erase(succsIter);
      } else {
        nodeSuccs.clear();
      }
    }
  }
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void
Graph<DataType, NodeBaseType>::_delDirectedArc(const NodeType * const from,
    const NodeType * const to)
{
  {
    am_iterator toIter = preds.find(to);

    CHECK_THROW((toIter != preds.end()),
        WrongArgumentException("Node: " + to->label+ " not in preds, while "
                                "deleting an arc") );

    NodeSetType & toPreds = toIter->second;
    toPreds.erase(from);
  }
  {
    am_iterator fromIter = succs.find(from);

    CHECK_THROW((fromIter != succs.end()),
        WrongArgumentException("Node: " + from->label+ " not in succs, while "
                                "deleting an arc") );

    NodeSetType & fromSuccs = fromIter->second;
    fromSuccs.erase(to);
  }
}


template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void
Graph<DataType, NodeBaseType>::_addDirectedArc(const NodeType * const from,
    const NodeType * const to)
{
  {
    am_iterator toIter = preds.find(to);

    CHECK_THROW((toIter != preds.end()),
        WrongArgumentException("Node: " + to->label+ " not in preds, while "
                                "adding an arc") );

    NodeSetType & toPreds = toIter->second;
    toPreds.insert(from);
  }
  {
    am_iterator fromIter = succs.find(from);

    CHECK_THROW((fromIter != succs.end()),
        WrongArgumentException("Node: " + from->label+ " not in succs, while "
                                "adding an arc") );

    NodeSetType & fromSuccs = fromIter->second;
    fromSuccs.insert(to);
  }
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void
Graph<DataType, NodeBaseType>::_delUndirectedArc(const NodeType * const n1,
    const NodeType * const n2)
{
  _delDirectedArc(n1,n2);
  _delDirectedArc(n2,n1);
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
  : baseGraph(new BaseGraph<DataType, NodeBaseType>(*old.baseGraph))
  , baseGraphIsPrivate(true)
{
  for(nl_c_iterator nodeIt = getListOfNodes().begin();
      nodeIt != getListOfNodes().end(); nodeIt++)
  {
    const NodeType * const node = &*nodeIt;

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
Graph<DataType, NodeBaseType>::Graph(BaseGraph<DataType, NodeBaseType> * old,
    const bool & copy)
  : baseGraph(copy ? new BaseGraph<DataType, NodeBaseType>(old) : old)
  , baseGraphIsPrivate(copy)
{
  for(nl_iterator nodeIt = getListOfNodes().begin();
      nodeIt != getListOfNodes().end(); nodeIt++)
  {
    const NodeType * const node = &*nodeIt;

    preds.insert(typename ArcsMap::value_type(node, NodeSetType() ));
    succs.insert(typename ArcsMap::value_type(node, NodeSetType() ));
  }
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE typename Graph<DataType, NodeBaseType>::NodeType *
Graph<DataType, NodeBaseType>::addNewNode(const string & _label,
    DataType _data)
{
  NodeType * node = NULL;
  if (baseGraphIsPrivate) {
    if (getMapOfNodes().count(_label)) {
      throw DuplicateLabelException("Label: '" + _label +
          "' already associated to a node in the graph");
    }

    node = baseGraph->_addNewNode(_label,_data);
  } else {
    node = this->checkLabel(_label, "Reference Graph was not updated");
  }

  preds.insert(typename ArcsMap::value_type(node, NodeSetType() ));
  succs.insert(typename ArcsMap::value_type(node, NodeSetType() ));

  return node;
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE void
Graph<DataType, NodeBaseType>::removeNode(const NodeType * const node)
{
  checkNodePtr(node, "Trying to remove a node that is not in the graph");

  if (baseGraphIsPrivate) {
    this->_removeNode(node);
  } else {
    this->_removeAllArcs(node, true);
  }
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE void
Graph<DataType, NodeBaseType>::removeNode(const string & _label)
{
  const string errorMsg = "Trying to remove a node that is not in the graph";
  const NodeType * const node = checkLabel(_label, errorMsg);

  if (baseGraphIsPrivate) {
    this->_removeNode(node);
  } else {
    this->_removeAllArcs(node, true);
  }
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE void
Graph<DataType, NodeBaseType>::clear()
{
  if (baseGraphIsPrivate) {
    baseGraph->clear();
  }
  preds.clear();
  succs.clear();
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE void
Graph<DataType, NodeBaseType>::addDirectedArc(const string & _from,
    const string & _to)
{
  const string errorMsgF = "Trying to add an arc from a node not in the graph";
  const string errorMsgT = "Trying to add an arc to a node not in the graph";

  _addDirectedArc( checkLabel(_from, errorMsgF), checkLabel(_to, errorMsgT) );
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE void
Graph<DataType, NodeBaseType>::addDirectedArc(const NodeType * const from,
    const NodeType * const to)
{
  const string errorMsgF = "Trying to add an arc from a node not in the graph";
  const string errorMsgT = "Trying to add an arc to a node not in the graph";

  checkNodePtr(from, errorMsgF);
  checkNodePtr(to, errorMsgT);

  _addDirectedArc(from, to);
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE void
Graph<DataType, NodeBaseType>::addUndirectedArc(const string & node1,
    const string & node2)
{
  const string errorMsg = "Trying to add an arc from (and to) a node not in the"
      " graph";

  _addUndirectedArc( checkLabel(node1, errorMsg), checkLabel(node2, errorMsg) );
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE void
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
INLINE void
Graph<DataType, NodeBaseType>::removeAllArcs(const string & node1)
{
  const string errorMsg = "Trying to remove arcs from (and to) a node not in "
      "the graph";

  _removeAllArcs(checkLabel(node1, errorMsg));
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE void
Graph<DataType, NodeBaseType>::removeAllArcs(const NodeType * const node1)
{
  const string errorMsg = "Trying to remove arcs from (and to) a node not in "
      "the graph";

  baseGraph->checkNodePtr(node1, errorMsg);
  _removeAllArcs(node1);
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE size_t
Graph<DataType, NodeBaseType>::inDegree(const NodeType * const node) const
{
  const string errorMsg = "Trying to get the In Degree of a node not in the "
      "graph";

  checkNodePtr(node, errorMsg);
  return _inDegree(node);
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE size_t
Graph<DataType, NodeBaseType>::inDegree(const string & _label) const
{
  const string errorMsg = "Trying to get the In Degree of a node not in the "
      "graph";

  return _inDegree( checkLabel( _label, errorMsg ) );
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE size_t
Graph<DataType, NodeBaseType>::outDegree(const NodeType * const node) const
{
  const string errorMsg = "Trying to get the Out Degree of a node not in the "
      "graph";

  checkNodePtr(node, errorMsg);
  return _outDegree(node);
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE size_t
Graph<DataType, NodeBaseType>::outDegree(const string & _label) const
{
  const string errorMsg = "Trying to get the Out Degree of a node not in the "
      "graph";

  return _inDegree( checkLabel( _label, errorMsg ) );
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE void
Graph<DataType, NodeBaseType>::makeVisitList(
    deque<const NodeType *> & visitList, map<const NodeType *, bool> & visited,
    const NodeType * const rootNode)
{
  const NodeType * node = rootNode;
  if (getListOfNodes().size()) {
    if (!node) {
      node = &*getListOfNodes().begin();
    }

    visited.insert(typename map<const NodeType *, bool>::value_type(node,true));

    const NodeSetType & succsSet = getSuccs(node);
    for(ns_iterator succ = succsSet.begin(); succ != succsSet.end();
        succ++)
    {
      if (visited.find(*succ) == visited.end()) {
        this->makeVisitList(visitList, visited, *succ);
      }
    }

    visitList.push_back(node);
  }
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE const typename Graph<DataType, NodeBaseType>::NodeSetType &
Graph<DataType, NodeBaseType>::getPreds(const NodeType * const node) const
{
  am_c_iterator predsIter = preds.find(node);
  if (predsIter != preds.end()) {
    return predsIter->second;
  } else {
    throw WrongArgumentException("ERROR: Could find node (label " + node->label
                                  + ") in predecessors\n");
  }
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE const typename Graph<DataType, NodeBaseType>::NodeSetType &
Graph<DataType, NodeBaseType>::getSuccs(const NodeType * const node) const
{
  am_c_iterator succsIter = succs.find(node);
  if (succsIter != succs.end()) {
    return succsIter->second;
  } else {
    throw WrongArgumentException("ERROR: Could find node (label " + node->label
                                  + ") in successive\n");
  }
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
INLINE bool
Graph<DataType, NodeBaseType>::areAdjacent(const NodeType * const n1,
    const NodeType * const n2) const
{
  am_c_iterator predsIter = preds.find(n1);
  am_c_iterator succsIter = succs.find(n1);

  if (predsIter != preds.end() && succsIter != succs.end()) {
    return (predsIter->second.count(n2) != 0)
        || (succsIter->second.count(n2) != 0);
  } else {
    throw WrongArgumentException("ERROR: Could find node (label " + n1->label
                                  + ") in predecessors or successive\n");
  }

}

#endif /* GRAPHS_H_ */
