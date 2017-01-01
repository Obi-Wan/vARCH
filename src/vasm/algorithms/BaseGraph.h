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

#include "macros.h"
#include "exceptions.h"

template<typename DataType>
class NodeGraph {
public:
  DataType data;

  std::string label;

  bool operator==(const NodeGraph<DataType> & other) const throw()
  {
    return (label == other.label) && (data == other.data);
  }
};

template<typename DataType, template<typename NodeDataType> class NodeBaseType = NodeGraph>
class BaseGraph {
public:
  typedef class NodeBaseType<DataType> NodeType;

  typedef class std::list<NodeType> NodeListType;
  typedef class std::map<const std::string, const NodeType * const> NodeMapType;

  typedef typename NodeListType::iterator       nl_iterator;
  typedef typename NodeListType::const_iterator nl_c_iterator;
  typedef typename NodeMapType::iterator        nm_iterator;
  typedef typename NodeMapType::const_iterator  nm_c_iterator;

protected:
  NodeListType nodes_list;
  NodeMapType nodes_map;

public:
  void checkNodePtr(const NodeType * const node, const std::string & errorMessage)
    const;

  NodeType * _addNewNode(const std::string & _label, DataType _data);
  void _removeNode(const NodeType * const node);

public:
  BaseGraph() { }
  BaseGraph(const BaseGraph<DataType, NodeBaseType> & old);

  void clear();

  NodeType * checkLabel(const std::string & _label, const std::string & _errorMsg);
  const NodeType * checkLabel(const std::string & _label, const std::string & _errorMsg)
      const;

  const NodeListType & getListOfNodes() const throw() { return nodes_list; }
  const NodeMapType & getMapOfNodes() const throw() { return nodes_map; }
};

////////////////////////////////////////////////////////////////////////////////
/// Class Graph
///
/// Private Members
////////////////////////////////////////////////////////////////////////////////

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void
BaseGraph<DataType, NodeBaseType>::checkNodePtr(const NodeType * const node,
    const std::string & errorMessage)
  const
{
  if (!node) { throw WrongArgumentException("Null pointer as node pointer"); }
  if (nodes_map.find(node->label) == nodes_map.end()) {
    DebugPrintf(("No label %s in map of nodes\n", node->label.c_str()));
    throw WrongArgumentException(errorMessage);
  }
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline typename BaseGraph<DataType, NodeBaseType>::NodeType *
BaseGraph<DataType, NodeBaseType>::_addNewNode(const std::string & _label,
    DataType _data)
{
  nodes_list.push_back(NodeType());
  NodeType & node = nodes_list.back();

  node.data = _data;
  node.label = _label;

  nodes_map.insert(typename NodeMapType::value_type(_label, &node));
  return &node;
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void
BaseGraph<DataType, NodeBaseType>::_removeNode(const NodeType * const node)
{
  nodes_map.erase(node->label);

  for(auto list_it = nodes_list.begin(); list_it != nodes_list.end(); list_it++)
  {
    if (*list_it == *node) {
      nodes_list.erase(list_it);
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
BaseGraph<DataType, NodeBaseType>::checkLabel(const std::string & _label,
    const std::string & errorMessage)
  const
{
  nm_c_iterator nodeIter = nodes_map.find(_label);

  if (nodeIter == nodes_map.end()) {
    DebugPrintf(("No label %s in map of nodes\n", _label.c_str()));
    throw WrongArgumentException(errorMessage);
  }

  return nodeIter->second;
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline typename BaseGraph<DataType, NodeBaseType>::NodeType *
BaseGraph<DataType, NodeBaseType>::checkLabel(const std::string & _label,
    const std::string & errorMessage)
{
  nm_iterator nodeIter = nodes_map.find(_label);

  if (nodeIter == nodes_map.end()) {
    DebugPrintf(("No label %s in map of nodes\n", _label.c_str()));
    throw WrongArgumentException(errorMessage);
  }

  return (NodeType *) nodeIter->second;
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
BaseGraph<DataType, NodeBaseType>::BaseGraph(const BaseGraph<DataType, NodeBaseType> & old)
  : nodes_list(old.nodes_list)
{
  for(auto nodeIt = nodes_list.begin(); nodeIt != nodes_list.end(); nodeIt++)
  {
    const NodeType * const node = &*nodeIt;
    nodes_map.insert(typename NodeMapType::value_type(node->label, node));
    DebugPrintf(("Inserted pair: %p, %s\n", node, node->label.c_str()));
  }
}

template<typename DataType, template<typename NodeDataType> class NodeBaseType>
inline void
BaseGraph<DataType, NodeBaseType>::clear()
{
  nodes_list.clear();
  nodes_map.clear();
}



#endif /* BASEGRAPH_H_ */
