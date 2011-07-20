/*
 * Graphs.cpp
 *
 *  Created on: 20/lug/2011
 *      Author: ben
 */

#include "Graphs.h"

void
InteferenceGraph::printInterferenceGraph() const
{
  for(nl_c_iterator nodeIt = this->listOfNodes.begin();
      nodeIt != this->listOfNodes.end(); nodeIt++)
  {
    const NodeType * const node = &*nodeIt;
    cout << "Node - pointer: " << node << ", label: " << node->label;
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
    cout << endl;
  }
  DebugPrintf(("Map of labels\n"));
  for(nm_c_iterator mapIter = this->mapOfNodes.begin();
      mapIter != this->mapOfNodes.end(); mapIter++)
  {
    DebugPrintf((" label: %s, pointer %p\n", mapIter->first.c_str(), mapIter->second));
  }
}

