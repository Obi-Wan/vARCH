/*
 * AssemFlowGraph.h
 *
 *  Created on: 19/lug/2011
 *      Author: ben
 */

#ifndef ASSEMFLOWGRAPH_H_
#define ASSEMFLOWGRAPH_H_

#include "../asm-function.h"
#include "../algorithms/FlowGraph.h"
#include "../algorithms/TempsMap.h"

class AssemFlowGraph : public FlowGraph<asm_statement *> {
  typedef map<asm_statement *, NodeFlowGraph<asm_statement *> * > StmtToNode;

  StmtToNode backReference;

  string buildLabel(const asm_statement * const stmt, const uint32_t & progr)
    const;

  TempsMap tempsMap;

  void _addNodesToGraph(asm_function & function);
  void _createArcs(const TableOfSymbols & functionSymbols);
  void _findUsesDefines();

  bool _argIsTemp(const asm_arg * const arg) const;
  bool _moveInstr(const vector<asm_arg *> & args, UIDMultiSetType & nodeUses,
      UIDMultiSetType & nodeDefs);
  bool _argIsDefined(const int & instruction, const size_t & argNum,
      const TypeOfArgument & argType) const;

public:
//  AssemFlowGraph();

  void populateGraph(asm_function & function);
  void applySelectedRegisters(const AssignedRegs & regs);

  virtual void clear();

  const TempsMap & getTempsMap() const throw() { return tempsMap; }
};

#endif /* ASSEMFLOWGRAPH_H_ */
