'''
Created on 07/lug/2011

@author: ben
'''

class GraphNode(object):
    def __init__(self):
        self.pred = []
        self.succ = []

    def addPred(self, _pred):
        self.pred.append(_pred)
    def addSucc(self, _succ):
        self.succ.append(_succ)

    def degIn(self):
        return len(self.pred)
    def degOut(self):
        return len(self.succ)
    
    def printPred(self):
        output = ""
        for pr in self.pred:
            output = output + " " + str(pr)
        return output
    def printSucc(self):
        output = ""
        for sc in self.succ:
            output = output + " " + str(sc)
        return output

class Graph(object):
    def __init__(self):
        self.nodes = []
        self.arcs = {}
        
    def newNode(self):
        self.nodes.append(GraphNode())
    
    def fromNodeToNode(self, node1, node2):
        self.nodes[node2].pred.append(node1)
        self.nodes[node1].succ.append(node2)

    def addArc(self, node1, node2):
        if node1 in self.arcs:
            self.arcs[node1].append(node2)
        else:
            self.arcs[node1] = [node2]
        if node2 in self.arcs:
            self.arcs[node2].append(node1)
        else:
            self.arcs[node2] = [node1]
            
    def _printNode(self, node):
        nodeObj = self.nodes[node]
        arcsStr = ""
        if node in self.arcs:
            for arc in self.arcs[node]:
                arcsStr = arcsStr + " " + str(arc)
        print("Node: %d\n  pred:%s\n  succ:%s\n  arcs:%s" % 
              ( node, nodeObj.printPred(), nodeObj.printSucc(), arcsStr ))

    def printGraph(self):
        for node in range(len(self.nodes)):
            self._printNode(node)
            print("")

class FlowGraph(Graph):
    def __init__(self):
        Graph.__init__(self)
        self.defs = []
        self.uses = []

    def newNode(self, _defs = [], _uses = []):
        Graph.newNode(self)
        self.defs.append([])
        self.defs[-1].extend(_defs)
        self.uses.append([])
        self.uses[-1].extend(_uses)
        
    def _printList(self, theList, node):
        output = ""
        for el in theList[node]:
            output = output + " " + el
        return output
        
    def printFlowGraph(self):
        for node in range(len(self.nodes)):
            self._printNode(node)
            print( "  uses:%s\n  defs:%s\n" %
                   ( self._printList(self.uses, node),
                     self._printList(self.defs, node)))

class LiveMap(object):
    def __init__(self):
        self.liveIn  = [ ]
        self.liveOut = [ ]

    def _printList(self, theList, node):
        output = ""
        for el in theList[node]:
            output = output + " " + el
        return output
        
    def buildMap(self, flowGraph):
        indexNodes = range(len(flowGraph.nodes))
        indexNodes.reverse()
        for numNode in indexNodes:
            self.liveIn.append([])
            self.liveOut.append([])
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
        for node in range(len(self.liveIn)):
            print("Node %d, live-in:%20s , live-out:%20s" %
                  ( node, self._printList(self.liveIn, node),
                    self._printList(self.liveOut, node)))

class InterferenceGraph(Graph):
    pass

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

gr.addArc(0, 2)

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

gr.addArc(0, 2)

gr.printFlowGraph()

lm = LiveMap()
lm.buildMap(gr)
lm.printMap()

