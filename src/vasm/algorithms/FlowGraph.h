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
//  FlowGraph() { }
//  FlowGraph(const FlowGraph<DataType> & other);

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
      cout << " T" << *ins;
    }
    cout << "\n   live-out (num: " << live_out->second.size() << "):";
    for(us_iterator outs = live_out->second.begin();
        outs != live_out->second.end(); outs++)
    {
      cout << " T" << *outs;
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
      cout << " T" << *useIt;
    }
    cout << "\n  Defs:";
    const UIDMultiSetType & nodeDefs = defs.find(node)->second;
    for(us_c_iterator defIt = nodeDefs.begin(); defIt != nodeDefs.end();
        defIt++)
    {
      cout << " T" << *defIt;
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

#endif /* FLOWGRAPH_H_ */
