'''
Created on 07/lug/2011

@author: ben
'''

class ListProxy(object):
        
    def _printList(self, theList):
        output = ""
        for el in theList:
            output = output + " " + str(el)
        return output

class GraphNode(ListProxy):
    def __init__(self, label):
        self.pred = []
        self.succ = []
        self.label = label

    def addPred(self, _pred):
        self.pred.append(_pred)
    def addSucc(self, _succ):
        self.succ.append(_succ)

    def degIn(self):
        return len(self.pred)
    def degOut(self):
        return len(self.succ)
    
    def printPred(self):
        return self._printList(self.pred)
    def printSucc(self):
        return self._printList(self.succ)

class Graph(ListProxy):
    def __init__(self):
        self.nodes = {}
        self.arcs = {}
        self.recycleNodes = []
        self.labels = {}
        
    def getNodes(self):
        return list(self.labels)
    
    def getNodeNum(self, label):
        return self.labels[label]
        
    def newNode(self, label = None):
        if label is not None and label in self.labels:
            raise ValueError("Label " + label + " already defined")
        nodeNum = -1
        if len(self.recycleNodes) > 0:
            nodeNum = min(self.recycleNodes)
            self.recycleNodes.remove(nodeNum)
        elif len(self.nodes) > 0:
            nodeNum = max(self.nodes) + 1
        else:
            nodeNum = 0
        if label is None:
            label = nodeNum
        self.nodes[nodeNum] = GraphNode(label)
        self.labels[label] = nodeNum
        return nodeNum

    def delNode(self, node):
        for otherNode in self.nodes:
             self.delRelationArc(node, otherNode)
        if isinstance(self.nodes[node], GraphNode):
            for pred in self.nodes[node].pred:
                 self.nodes[pred].succ.remove(node)
            for succ in self.nodes[node].succ:
                 self.nodes[succ].pred.remove(node)
        del self.labels[self.nodes[node].label]
        del self.nodes[node]
        self.recycleNodes.append(node)
    
    def fromNodeToNode(self, node1, node2):
        self.nodes[node2].pred.append(node1)
        self.nodes[node1].succ.append(node2)

    def addRelationArc(self, node1, node2):
        if node1 in self.arcs:
            if node2 not in self.arcs[node1]:
                self.arcs[node1].append(node2)
        else:
            self.arcs[node1] = [node2]
        if node2 in self.arcs:
            if node1 not in self.arcs[node2]:
                self.arcs[node2].append(node1)
        else:
            self.arcs[node2] = [node1]

    def delRelationArc(self, node1, node2):
        if node1 in self.arcs:
            if node2 in self.arcs[node1]:
                self.arcs[node1].remove(node2)
        if node2 in self.arcs:
            if node1 in self.arcs[node2]:
                self.arcs[node2].remove(node1)
            
    def _printNode(self, node):
        nodeObj = self.nodes[node]
        arcsStr = ""
        if node in self.arcs:
            for arc in self.arcs[node]:
                arcsStr = arcsStr + " " + str(arc)
        print("Node: %s" % str(node))
        if isinstance(nodeObj, GraphNode):
            print("  label: %s\n  pred:%s\n  succ:%s" % 
                  ( str(nodeObj.label), nodeObj.printPred(),
                    nodeObj.printSucc() ))
        else:
            print("  label: %s" % str(nodeObj))
        print("  arcs:%s" % arcsStr) 

    def printGraph(self):
        for node in self.nodes:
            self._printNode(node)
            print("")

class FlowGraph(Graph):
    def __init__(self):
        Graph.__init__(self)
        self.defs = {}
        self.uses = {}

    def newNode(self, _defs = [], _uses = [], label = None):
        nodeNum = Graph.newNode(self, label)
        self.defs[nodeNum] = []
        self.defs[nodeNum].extend(_defs)
        self.uses[nodeNum] = []
        self.uses[nodeNum].extend(_uses)
        return nodeNum
        
    def printFlowGraph(self):
        for node in self.nodes:
            self._printNode(node)
            print( "  uses:%s\n  defs:%s\n" %
                   ( self._printList(self.uses[node]),
                     self._printList(self.defs[node])))

class LiveMap(ListProxy):
    def __init__(self):
        self.liveIn  = { }
        self.liveOut = { }

    def _makeVisitList(self, flowGraph, rootNode, visited, order):
        visited[rootNode] = True
        for succ in flowGraph.nodes[rootNode].succ:
            if succ not in visited:
                self._makeVisitList(flowGraph, succ, visited, order)
        order.append(rootNode)
        return order
        
        
    def buildMap(self, flowGraph):
        indexNodes = self._makeVisitList(flowGraph, min(flowGraph.nodes), {}, [])
        print("Visit list (ordered): %s" % self._printList(indexNodes))
        for numNode in indexNodes:
            self.liveIn[numNode] = []
            self.liveOut[numNode] = []
        for numNode in indexNodes:
            self.liveIn[numNode].extend(flowGraph.uses[numNode])
        while True:
            modified = False
            for numNode in indexNodes:
                for live_in in self.liveIn[numNode]:
                    for pred in flowGraph.nodes[numNode].pred:
                        if live_in not in self.liveOut[pred]:
                            self.liveOut[pred].append(live_in)
                            modified = True
                for live_out in self.liveOut[numNode]:
                    if (live_out not in self.liveIn[numNode]
                         and live_out not in flowGraph.defs[numNode]):
                        self.liveIn[numNode].append(live_out)
                        modified = True
            if not modified: break 
            
    def printMap(self):
        for node in self.liveIn:
            print("Node %d, live-in:%20s , live-out:%20s" %
                  ( node, self._printList(self.liveIn[node]),
                    self._printList(self.liveOut[node])))
        print("")

class InterferenceGraph(Graph):
    def __init__(self):
        Graph.__init__(self)
        
    def _buildInterference(self, flowGraph, liveMap):
        print("Interference for Nodes: %s" % self._printList(flowGraph.nodes))
        for node in flowGraph.nodes:
            for var in liveMap.liveIn[node]:
                if var not in self.getNodes():
                    #self.nodes[var] = var
                    self.newNode(var)
        for node in flowGraph.nodes:
            for var in liveMap.liveOut[node]:
                varNum = -1
                if var not in self.getNodes():
                    #self.nodes[var] = var
                    varNum = self.newNode(var)
                else:
                    varNum = self.getNodeNum(var)
                for defs in flowGraph.defs[node]:
                    if defs is not var:
                        defNum = self.getNodeNum(defs)
                        self.addRelationArc(varNum, defNum)
                    
class Liveness(InterferenceGraph):
    def __init__(self):
        InterferenceGraph.__init__(self)

    def buildFromFlow(self, flowGraph, liveMap):
        self._buildInterference(flowGraph, liveMap)

# esempio interno

gr = FlowGraph()
gr.newNode(['b', 'd'],[])
gr.newNode(['a'],['b', 'd'])
gr.newNode(['c'],['b', 'a'])
gr.newNode(['d'],[])
gr.newNode(['b'],['c', 'd'])

gr.fromNodeToNode(0, 1)
gr.fromNodeToNode(1, 2)
gr.fromNodeToNode(2, 3)
gr.fromNodeToNode(3, 4)

gr.addRelationArc(0, 2)

gr.printFlowGraph()

lm = LiveMap()
lm.buildMap(gr)
lm.printMap()

# esempio appel

gr = FlowGraph()
gr.newNode(['a'],[])
gr.newNode(['b'],['a'])
gr.newNode(['c'],['c', 'b'])
gr.newNode(['a'],['b'])
gr.newNode([],['a'])
gr.newNode([],['c'])

gr.fromNodeToNode(0, 1)
gr.fromNodeToNode(1, 2)
gr.fromNodeToNode(2, 3)
gr.fromNodeToNode(3, 4)
gr.fromNodeToNode(4, 5)

gr.fromNodeToNode(4, 1)

gr.printFlowGraph()

lm = LiveMap()
lm.buildMap(gr)
lm.printMap()

liveness = Liveness()
liveness.buildFromFlow(gr, lm)
liveness.printGraph()

