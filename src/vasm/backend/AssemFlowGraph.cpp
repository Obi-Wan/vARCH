/*
 * AssemFlowGraph.cpp
 *
 *  Created on: 19/lug/2011
 *      Author: ben
 */

#include "AssemFlowGraph.h"

#include "exceptions.h"
#include "CpuDefinitions.h"
#include "StaticLinker.h"

#include <sstream>

using namespace std;

inline string
AssemFlowGraph::buildStmtLabel(const asm_statement * const stmt,
    const uint32_t & progr) const
{
  stringstream stream;
  stream.width(10);
  stream.fill('0');
  stream << progr;
  stream.width(0);
  stream.fill(' ');
  stream << " " << stmt->toString();
  return stream.str();
}

inline void
AssemFlowGraph::_addNodesToGraph(asm_function & function)
{
  uint32_t progr = 0; // progressive number of instruction
  ListOfStmts & stmts = function.stmts;
  for(ListOfStmts::iterator stmtIt = stmts.begin(); stmtIt != stmts.end();
      stmtIt++)
  {
    asm_statement * stmt = *stmtIt;
    this->addNewNode(buildStmtLabel(stmt, progr++), stmt);

    NodeType * const node = &listOfNodes.back();
    backReference.insert(StmtToNode::value_type(stmt, node));
  }
}

inline void
AssemFlowGraph::_createArcs(const TableOfSymbols & functionSymbols)
{
  for(nl_iterator nodeIt = listOfNodes.begin(); nodeIt != listOfNodes.end();
      nodeIt++)
  {
    const NodeType * const node = &*nodeIt;
    nl_iterator nextNodeIt = nodeIt;
    nextNodeIt++;

    asm_statement * stmt = node->data;
    if (stmt->getType() == ASM_LABEL_STATEMENT) {
      if (nextNodeIt != listOfNodes.end()) {
        DebugPrintf(("Adding arc to next node\n"));
        addDirectedArc(node, &*nextNodeIt);
      }
    } else if (stmt->getType() == ASM_INSTRUCTION_STATEMENT
                || stmt->getType() == ASM_FUNCTION_CALL)
    {
      asm_instruction_statement * i_stmt = (asm_instruction_statement *) stmt;
      switch(i_stmt->instruction) {
        case IFJ:
        case IFNJ:
        case TCJ:
        case TZJ:
        case TOJ:
        case TNJ:
        case TSJ:
        case IFEQJ:
        case IFNEQJ:
        case IFLOJ:
        case IFMOJ:
        case IFLEJ:
        case IFMEJ: {
          if (nextNodeIt != listOfNodes.end()) {
            DebugPrintf(("Adding arc to next node\n"));
            addDirectedArc(node, &*nextNodeIt);
          }
        }
        case JMP: {
          asm_arg * arg = i_stmt->args[0];
          if (arg->getType() == ASM_LABEL_ARG) {
            asm_label_arg * l_arg = (asm_label_arg *) arg;
            asm_statement * pointedStmt =
                functionSymbols.getStmt(l_arg->label);

            if (!pointedStmt) {
              throw WrongArgumentException(
                  "Not possible to jump to labels outside of function body "
                  "(in auto-register mode)");
            }

            DebugPrintf(("Adding arc to jumped node\n"));
            const NodeType * const pointedNode = backReference[pointedStmt];
            addDirectedArc(node, pointedNode);

          } else {
            throw WrongArgumentException(
                "Not possible to use Immediate Arguments in jumps "
                "(in auto-register mode)");
          }
          break;
        }
        default: {
          if (nextNodeIt != listOfNodes.end()) {
            DebugPrintf(("Adding arc to next node\n"));
            addDirectedArc(node, &*nextNodeIt);
          }
          break;
        }
      }
    }
  }
}

inline void
AssemFlowGraph::_addToSet(UIDMultiSetType & nodeSet, const uint32_t &shiftedUID)
{
  if (!StaticLinker::shiftedIsSpecialReg(shiftedUID))
  {
    nodeSet.insert(shiftedUID);
  }
}

inline bool
AssemFlowGraph::_moveInstr(const vector<asm_arg *> & args,
    UIDMultiSetType & nodeUses, UIDMultiSetType & nodeDefs)
{
  const bool arg0_temp = args[0]->isTemporary();
  const bool arg1_temp = args[1]->isTemporary();

  bool isMove = arg0_temp && arg1_temp;

  if (arg0_temp || args[0]->isReg()) {
    asm_immediate_arg * arg = (asm_immediate_arg *) args[0];
    const uint32_t shiftedTempUID = StaticLinker::shiftArgUID(arg, arg0_temp);
//      // Add to temps map
//      tempsMap.putTemp( shiftedTempUID, true);
    // Add to uses
    _addToSet(nodeUses, shiftedTempUID );
    // Test if it should be in Defines, too
    switch(arg->type) {
      case REG_PRE_INCR:
      case REG_PRE_DECR:
      case REG_POST_INCR:
      case REG_POST_DECR:
      case ADDR_IN_REG_PRE_INCR:
      case ADDR_IN_REG_PRE_DECR:
      case ADDR_IN_REG_POST_INCR:
      case ADDR_IN_REG_POST_DECR: {
        _addToSet(nodeDefs, shiftedTempUID );
        isMove = false;
      }
      default: break;
    }
  }
  if (arg1_temp || args[1]->isReg()) {
    asm_immediate_arg * arg = (asm_immediate_arg *) args[1];
    const uint32_t shiftedTempUID = StaticLinker::shiftArgUID(arg, arg1_temp);
    // Add to defs
    _addToSet(nodeDefs, shiftedTempUID );
  }
  return isMove;
}

inline bool
AssemFlowGraph::_argIsDefined(const int & instruction, const size_t & argNum,
    const TypeOfArgument & argType)
  const
{
  switch (instruction) {
    case NOT:
    case INCR:
    case DECR:
    case COMP2:
    case LSH:
    case RSH: {
      return true;
    }
    case ADD:
    case MULT:
    case SUB:
    case DIV:
    case QUOT:
    case AND:
    case OR:
    case XOR:
    case GET: {
      if (argNum == 1) return true;
    }
    default: {
      switch(argType) {
        case REG_PRE_INCR:
        case REG_PRE_DECR:
        case REG_POST_INCR:
        case REG_POST_DECR:
        case ADDR_IN_REG_PRE_INCR:
        case ADDR_IN_REG_PRE_DECR:
        case ADDR_IN_REG_POST_INCR:
        case ADDR_IN_REG_POST_DECR: {
          return true;
        }
        default:
          break;
      }
      return false;
    }
  }
}

inline void
AssemFlowGraph::_findUsesDefines()
{
  for(nl_iterator nodeIt = listOfNodes.begin(); nodeIt != listOfNodes.end();
      nodeIt++)
  {
    NodeType * const node = &*nodeIt;
    asm_statement * stmt = node->data;
    if (stmt->isInstruction()) {
      asm_instruction_statement * i_stmt = (asm_instruction_statement *) stmt;

      UIDMultiSetType & nodeUses = uses[node];
      UIDMultiSetType & nodeDefs = defs[node];

      if (stmt->getType() == ASM_INSTRUCTION_STATEMENT) {
        // Special treatment of "move" instruction
        if (i_stmt->instruction == MOV) {
          node->isMove = _moveInstr(i_stmt->args, nodeUses, nodeDefs);
        } else {
          // Other instructions
          for(size_t argNum = 0; argNum < i_stmt->args.size(); argNum++)
          {
            const bool isTemp = i_stmt->args[argNum]->isTemporary();
            if ( isTemp || i_stmt->args[argNum]->isReg() ) {
              asm_immediate_arg * arg =
                                      (asm_immediate_arg *)i_stmt->args[argNum];
              const uint32_t shiftedTempUID =
                                        StaticLinker::shiftArgUID(arg, isTemp);

              // uses
              _addToSet( nodeUses, shiftedTempUID );

              // defines
              if (_argIsDefined(i_stmt->instruction, argNum, arg->type)) {
                _addToSet( nodeDefs, shiftedTempUID );
              }
            }
          } // End loop arguments
        }
      } else if (stmt->getType() == ASM_FUNCTION_CALL) {
        const asm_function_call * f_stmt = (const asm_function_call *) stmt;
        // Surely it defines the caller-save registers
        for(uint32_t numReg = 0; numReg < STD_CALLEE_SAVE; numReg++)
        {
          const uint32_t shiftedTempUID =
                                      StaticLinker::shiftArgUID(numReg, false);
          _addToSet( nodeDefs, shiftedTempUID );
        }
        // It uses the arguments
        for(ConstListOfParams::const_iterator parIt = f_stmt->parameters.begin();
            parIt != f_stmt->parameters.end(); parIt++)
        {
          const asm_function_param * par = *parIt;
          const uint32_t shiftedTempUID =
                          StaticLinker::shiftArgUID(par->content.regNum, false);
          _addToSet( nodeUses, shiftedTempUID );
        }
      } else if (stmt->getType() == ASM_RETURN_STATEMENT) {
        // Probably it uses the callee-save registers, and returned element
        for(uint32_t regNum = STD_CALLEE_SAVE; regNum < NUM_REGS; regNum++)
        {
          const uint32_t shiftedTempUID =
                                      StaticLinker::shiftArgUID(regNum, false);
          _addToSet( nodeUses, shiftedTempUID );
        }
      }
    }
  }
}

void
AssemFlowGraph::clear()
{
  FlowGraph<asm_statement *>::clear();
  backReference.clear();
  tempsMap.clear();
}

void
AssemFlowGraph::populateGraph(asm_function & function)
{
  DebugPrintf(("  - Populating AssemGraph\n"));
  // import nodes from assembler code
  _addNodesToGraph(function);

  DebugPrintf(("  - Adding Arcs\n"));
  // Assigns relations
  _createArcs(function.localSymbols);

  DebugPrintf(("  - Finding Uses and Defs\n"));
  // Finds uses and defines
  _findUsesDefines();
  DebugPrintf(("  - Done\n"));
}

void
AssemFlowGraph::applySelectedRegisters(const AssignedRegs & regs)
{
  for(nl_iterator nodeIt = listOfNodes.begin(); nodeIt != listOfNodes.end();
      nodeIt++)
  {
    if (nodeIt->data->getType() == ASM_INSTRUCTION_STATEMENT) {
      asm_instruction_statement * stmt =
                                    (asm_instruction_statement *) nodeIt->data;
      for(vector<asm_arg *>::iterator argIt = stmt->args.begin();
          argIt != stmt->args.end(); argIt++)
      {
        if ((*argIt)->isTemporary()) {
          asm_immediate_arg * arg = (asm_immediate_arg *) *argIt;
          const uint32_t temp_uid = StaticLinker::shiftArgUID(arg, true);

          AssignedRegs::const_iterator reg = regs.find(temp_uid);
          if (reg == regs.end()) {
            throw WrongArgumentException(
                "A temporary in instruction was not considered! ("
                + tempsMap.getLabel(temp_uid) + ")");
          }
          if (reg->second) {
            arg->content.tempUID = reg->second -1;
            arg->isTemp = false;
          } else {
            throw WrongArgumentException(
                "Pending Spills! not using a solution!");
          }
        }
      }
    }
  }
}



