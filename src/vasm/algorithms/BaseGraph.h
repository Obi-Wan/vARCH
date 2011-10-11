/*
 * BaseGraph.h
 *
 *  Created on: 11/ott/2011
 *      Author: ben
 */

#ifndef BASEGRAPH_H_
#define BASEGRAPH_H_

#include <map>
#include <list>
#include <string>

using namespace std;

#include "macros.h"
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

template<typename DataType, template<typename NodeDataType> class NodeBaseType = NodeGraph>
class BaseGraph {
public:
  typedef class NodeBaseType<DataType> NodeType;

  typedef class list<NodeType>                            NodeListType;
  typedef class map<const string, const NodeType * const> NodeMapType;

  typedef typename NodeListType::iterator       nl_iterator;
  typedef typename NodeListType::const_iterator nl_c_iterator;
  typedef typename NodeMapType::iterator        nm_iterator;
  typedef typename NodeMapType::const_iterator  nm_c_iterator;

protected:
  NodeListType listOfNodes;

  NodeMapType mapOfNodes;

  void checkNodePtr(const NodeType * const node, const string & errorMessage)
    const;

  NodeType * _addNewNode(const string & _label, DataType _data);
  void _removeNode(const NodeType * const node);

public:
  BaseGraph() { }
  BaseGraph(const BaseGraph<DataType, NodeBaseType> & old);

  NodeType * checkLabel(const string & _label, const string & _errorMsg);
  const NodeType * checkLabel(const string & _label, const string & _errorMsg)
      const;

  const NodeListType & getListOfNodes() const throw() { return listOfNodes; }
  const NodeMapType & getMapOfNodes() const throw() { return mapOfNodes; }
};

////////////////////////////////////////////////////////////////////////////////
/// Class Graph
///
/// Private Members
////////////////////////////////////////////////////////////////////////////////

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void
BaseGraph<DataType, NodeBaseType>::checkNodePtr(const NodeType * const node,
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
inline typename BaseGraph<DataType, NodeBaseType>::NodeType *
BaseGraph<DataType, NodeBaseType>::_addNewNode(const string & _label,
    DataType _data)
{
  listOfNodes.push_back(NodeType());
  NodeType & node = listOfNodes.back();

  node.data = _data;
  node.label = _label;

  mapOfNodes.insert(typename NodeMapType::value_type(_label, &node));
  return &node;
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void
BaseGraph<DataType, NodeBaseType>::_removeNode(const NodeType * const node)
{
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

////////////////////////////////////////////////////////////////////////////////
/// Class Graph
///
/// Public Members
////////////////////////////////////////////////////////////////////////////////

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline const typename BaseGraph<DataType, NodeBaseType>::NodeType *
BaseGraph<DataType, NodeBaseType>::checkLabel(const string & _label,
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
inline typename BaseGraph<DataType, NodeBaseType>::NodeType *
BaseGraph<DataType, NodeBaseType>::checkLabel(const string & _label,
    const string & errorMessage)
{
  nm_iterator nodeIter = mapOfNodes.find(_label);

  if (nodeIter == mapOfNodes.end()) {
    DebugPrintf(("No label %s in map of nodes\n", _label.c_str()));
    throw WrongArgumentException(errorMessage);
  }

  return (NodeType *) nodeIter->second;
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
BaseGraph<DataType, NodeBaseType>::BaseGraph(const BaseGraph<DataType, NodeBaseType> & old)
  : listOfNodes(old.listOfNodes)
{
  for(nl_iterator nodeIt = listOfNodes.begin(); nodeIt != listOfNodes.end();
      nodeIt++)
  {
    const NodeType * const node = &*nodeIt;
    mapOfNodes.insert(typename NodeMapType::value_type(node->label,node));
    DebugPrintf(("Inserted pair: %p, %s\n", node, node->label.c_str()));
  }
}

#endif /* BASEGRAPH_H_ */
