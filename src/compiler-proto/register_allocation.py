'''
Created on 11/lug/2011

@author: ben
'''

import liveness_analysis
from copy import deepcopy

#hard_nodes = ["b", "c", "d", "e", "f", "g", "h", "j", "k", "m"]
hard_nodes = ["h", "g", "d", "e", "f", "c", "b", "j", "k", "m"]
hard_costs = [ ("h", 2), ("g", 2), ("d", 2), ("e", 2), ("f", 2), ("c", 2),
               ("b", 2), ("j", 4), ("k", 3), ("m", 2)]
hard_rels = [
              ("j", "f"), ("j", "e"), ("j", "k"), ("j", "d"), ("j", "h"),
              ("j", "g"), ("f", "m"), ("f", "e"), ("m", "e"), ("m", "b"),
              ("m", "c"), ("m", "d"), ("b", "e"), ("b", "k"), ("b", "c"),
              ("b", "d"), ("k", "d"), ("k", "g"), ("h", "g")
            ]
hard_moves = [ ("j", "b"), ("d", "c") ]
hard_interf = liveness_analysis.Liveness()
hard_interf.buildFromGiven(hard_nodes, hard_rels, hard_costs, hard_moves)

class SimpleAllocator(object):
    def __init__(self, numRegs):
        self.stackRegs = []
        self.numRegs = numRegs
        self.allocatedRegs = {}
        
    def _printStack(self, interfGraph):
        print("Stack in reverse order (Registers Count: %d)" % self.numRegs)
        for node in self.stackRegs:
            args = (interfGraph.nodes[node[0]].label, ) + node
            print("Label: %2s, Node: %d, Potential Spill: %5s, Degree: %d"
                   % args )

    def _printAllocated(self, interfGraph):
        print("Allocated Registers (Registers Count: %d)" % self.numRegs)
        for node, reg in self.allocatedRegs.iteritems():
            args = (interfGraph.nodes[node].label, node, str(reg))
            print("Label: %2s, Node: %d, Register: %12s" % args )
    
    def allocate(self, interfGraph):
        workGraph = deepcopy(interfGraph)
        while len(workGraph.nodes) > 0:
            notSignificNodes = []
            significNodes = []
            for node in workGraph.nodes:
                if workGraph.nodeDegree(node) < self.numRegs:
                    notSignificNodes.append((node, workGraph.nodeDegree(node)))
                else:
                    significNodes.append((node, workGraph.nodeDegree(node)))

            max = (-1, -1)
            if len(notSignificNodes) > 0:
                for node in notSignificNodes:
                    if node[1] > max[1]:
                        max = node
                self.stackRegs.append((max[0], False, max[1]))
            else:
                for node in significNodes:
                    if node[1] > max[1]:
                        max = node
                self.stackRegs.append((max[0], True, max[1]))
            workGraph.delNode(max[0])
        self._printStack(interfGraph)
        
        while len(self.stackRegs) > 0:
            topStack = self.stackRegs[-1]
            leftRegs = range(1, self.numRegs+1)
            for adj in interfGraph.arcs[topStack[0]]:
                if ( adj in self.allocatedRegs
                     and self.allocatedRegs[adj] in leftRegs ):
                    leftRegs.remove(self.allocatedRegs[adj])
            if len(leftRegs) > 0:
                self.allocatedRegs[topStack[0]] = leftRegs[0]
            else:
                self.allocatedRegs[topStack[0]] = "Actual Spill"
            self.stackRegs.remove(topStack)
        self._printAllocated(interfGraph)
    
    def printColoredGraph(self, interfGraph):
        workGraph = deepcopy(interfGraph)
        for node in self.allocatedRegs:
            workGraph.nodes[node].label = ( workGraph.nodes[node].label + " "
                                             + str(self.allocatedRegs[node]) )
        workGraph.printGraph()
    
    def isSolution(self):
        for node in self.allocatedRegs:
            if self.allocatedRegs[node] == "Actual Spill": return False
        else: return True

sa = SimpleAllocator(4)
sa.allocate(hard_interf)
sa.printColoredGraph(hard_interf)
print("Found a solution: %s" % sa.isSolution())
