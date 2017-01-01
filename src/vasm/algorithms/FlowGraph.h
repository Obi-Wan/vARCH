/*
 * FlowGraph.h
 *
 *  Created on: 01/ago/2011
 *      Author: ben
 */

#ifndef FLOWGRAPH_H_
#define FLOWGRAPH_H_

#include "Graphs.h"

template<typename DataType>
class NodeFlowGraph : public NodeGraph<DataType> {
public:
  bool isMove;

  bool operator==(const NodeFlowGraph<DataType> & other) const throw()
  {
    return NodeGraph<DataType>::operator ==(other) && (isMove == other.isMove);
  }
};

template<typename DataType>
struct LiveMap {
  typedef class NodeGraph<DataType> NodeType;

  typedef class std::set<uint32_t> UIDSetType;
  typedef class std::map<const NodeType *, UIDSetType> UIDsMap;

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
  typedef typename Graph<DataType, NodeFlowGraph>::NodeType NodeType;
  typedef typename Graph<DataType, NodeFlowGraph>::NodeSetType NodeSetType;
  typedef typename Graph<DataType, NodeFlowGraph>::NodeListType NodeListType;
  typedef typename Graph<DataType, NodeFlowGraph>::NodeMapType NodeMapType;
  typedef typename Graph<DataType, NodeFlowGraph>::ArcsMap ArcsMap;

  typedef typename Graph<DataType, NodeFlowGraph>::ns_iterator    ns_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::ns_c_iterator  ns_c_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::nl_iterator    nl_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::nl_c_iterator  nl_c_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::nm_iterator    nm_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::nm_c_iterator  nm_c_iterator;
  typedef typename Graph<DataType, NodeFlowGraph>::am_iterator    am_iterator;

  // Newly Defined Types
  typedef class std::multiset<uint32_t> UIDMultiSetType;
  typedef class std::map<const NodeType *, UIDMultiSetType> UIDsMSMap;

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
//  FlowGraph() { }
//  FlowGraph(const FlowGraph<DataType> & other);
  virtual ~FlowGraph() { }

  virtual NodeType * addNewNode(const std::string & _label, DataType _data);
  virtual void removeNode(const std::string & _label);
  virtual void removeNode(const NodeType * const node);

  virtual void clear();

//  size_t numUses(const uint32_t & uid) const;
//  size_t numUses(const uint32_t & uid, const std::string & _node) const;
//  size_t numUses(const uint32_t & uid, const NodeType * const node) const;
//
//  size_t numDefs(const uint32_t & uid) const;
//  size_t numDefs(const uint32_t & uid, const std::string & _node) const;
//  size_t numDefs(const uint32_t & uid, const NodeType * const node) const;

  void populateLiveMap(LiveMap<DataType> & liveMap);

  const UIDsMSMap & getUses() const throw() { return uses; }
  const UIDsMSMap & getDefs() const throw() { return defs; }

  void printFlowGraph() const;
};


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
    std::cout << "Node " << live_in->first->label << std::endl
        << "   live-in  (num: " << live_in->second.size() << "):";
    for(auto & live_in_el : live_in->second) { std::cout << " T" << live_in_el; }
    std::cout << "\n   live-out (num: " << live_out->second.size() << "):";
    for(auto & live_out_el : live_out->second.end()) { std::cout << " T" << live_out_el; }
    std::cout << std::endl;
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
  const auto & usesIter = uses.find(node);
  return usesIter->second.count(uid);
}

template<typename DataType>
size_t
FlowGraph<DataType>::_numDefs(const uint32_t & uid, const NodeType * const node)
  const
{
  const auto & defsIter = defs.find(node);
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
typename FlowGraph<DataType>::NodeType *
FlowGraph<DataType>::addNewNode(const std::string & _label, DataType _data)
{
  NodeType * const node =
                      Graph<DataType, NodeFlowGraph>::addNewNode(_label, _data);
  _addNewUseDefRecords(node);

  return node;
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
FlowGraph<DataType>::removeNode(const std::string & _label)
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
  typedef typename std::deque<const NodeType *>::iterator        nd_iterator;
  typedef typename std::deque<const NodeType *>::const_iterator  nd_c_iterator;

  typedef class std::set<uint32_t> UIDSetType;
  typedef class std::map<const NodeType *, UIDSetType> UIDsMap;

  // Reset live map
  liveMap.liveIn.clear();
  liveMap.liveOut.clear();

  for(nl_c_iterator listIter = this->getListOfNodes().begin();
      listIter != this->getListOfNodes().end(); listIter++)
  {
    const NodeType * const node = &*listIter;
    const UIDMultiSetType & nodeUses = uses[node];
    liveMap.liveIn.insert(
        typename UIDsMap::value_type(node, UIDSetType(nodeUses.begin(),
                                                             nodeUses.end())) );
    liveMap.liveOut.insert(
        typename UIDsMap::value_type(node, UIDSetType()) );
  }

  std::deque<const NodeType *> visitList;
  std::map<const NodeType *, bool> visited;

  this->makeVisitList(visitList, visited);

#ifdef DEBUG
  uint32_t iter = 0;
#endif
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

          if ( !outPred.count(*live_in) ) {
            DebugPrintf(( "Adding Live-Out: T%05d to node %s\n",
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
        if ( !nodeLiveIn.count(*live_out) && !defs[node].count(*live_out) )
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
  for(nl_c_iterator nodeIt = this->getListOfNodes().begin();
      nodeIt != this->getListOfNodes().end(); nodeIt++)
  {
    const NodeType * const node = &*nodeIt;
    std::cout << "Node - pointer: " << node << "\n  label: " << node->label
        << "\n  isMove: " << std::boolalpha << node->isMove;
    std::cout << "\n  Preds:";
    const NodeSetType & nodePreds = this->preds.find(node)->second;
    for(ns_c_iterator predIt = nodePreds.begin(); predIt != nodePreds.end();
        predIt++)
    {
      std::cout << " " << *predIt;
    }
    std::cout << "\n  Succs:";
    const NodeSetType & nodeSuccs = this->succs.find(node)->second;
    for(ns_c_iterator succIt = nodeSuccs.begin(); succIt != nodeSuccs.end();
        succIt++)
    {
      std::cout << " " << *succIt;
    }
    std::cout << "\n  Uses:";
    const UIDMultiSetType & nodeUses = uses.find(node)->second;
    for(us_c_iterator useIt = nodeUses.begin(); useIt != nodeUses.end();
        useIt++)
    {
      std::cout << " T" << *useIt;
    }
    std::cout << "\n  Defs:";
    const UIDMultiSetType & nodeDefs = defs.find(node)->second;
    for(us_c_iterator defIt = nodeDefs.begin(); defIt != nodeDefs.end();
        defIt++)
    {
      std::cout << " T" << *defIt;
    }
    std::cout << std::endl;
  }
  DebugPrintf(("Map of labels\n"));
  for(nm_c_iterator mapIter = this->getMapOfNodes().begin();
      mapIter != this->getMapOfNodes().end(); mapIter++)
  {
    DebugPrintf((" label: %s, pointer %p\n", mapIter->first.c_str(), mapIter->second));
  }
}

#endif /* FLOWGRAPH_H_ */
