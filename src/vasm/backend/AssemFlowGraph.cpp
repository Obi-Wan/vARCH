/*
 * AssemFlowGraph.cpp
 *
 *  Created on: 19/lug/2011
 *      Author: ben
 */

#include "AssemFlowGraph.h"

#include "exceptions.h"

#include <sstream>

using namespace std;

bool
AssemFlowGraph::_argIsTemp(const asm_arg * const arg) const
{
  return (arg->getType() == ASM_IMMEDIATE_ARG
          && ((const asm_immediate_arg * const)arg)->isTemp);
}

string
AssemFlowGraph::buildLabel(const asm_statement * const stmt,
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
  vector<asm_statement *> & stmts = function.stmts;
  for(vector<asm_statement *>::iterator stmtIt = stmts.begin();
      stmtIt != stmts.end(); stmtIt++)
  {
    asm_statement * stmt = *stmtIt;
    this->addNewNode(buildLabel(stmt, progr++), stmt);

    NodeType * const node = &listOfNodes.back();
    backReference.insert(StmtToNode::value_type(stmt, node));
  }
}

inline void
AssemFlowGraph::_createArcs(const TableOfSymbols & functionSymbols)
{
  for(size_t nodeNum = 0; nodeNum < listOfNodes.size(); nodeNum++)
  {
    const NodeType * const node = &listOfNodes[nodeNum];
    asm_statement * stmt = node->data;
    if (stmt->getType() == ASM_LABEL_STATEMENT) {
      if (nodeNum < (listOfNodes.size() - 1)) {
        DebugPrintf(("Adding arc to next node\n"));
        addDirectedArc(node, &listOfNodes[nodeNum+1]);
      }
    } else if (stmt->getType() == ASM_INSTRUCTION_STATEMENT) {
      asm_instruction_statement * i_stmt = (asm_instruction_statement *) stmt;
      switch(i_stmt->instruction) {
        case IFJ:
        case IFNJ:
        case IFEQJ:
        case IFNEQJ:
        case IFLOJ:
        case IFMOJ:
        case IFLEJ:
        case IFMEJ: {
          if (nodeNum < (listOfNodes.size() - 1)) {
            DebugPrintf(("Adding arc to next node\n"));
            addDirectedArc(node, &listOfNodes[nodeNum+1]);
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
          if (nodeNum < (listOfNodes.size() - 1)) {
            DebugPrintf(("Adding arc to next node\n"));
            addDirectedArc(node, &listOfNodes[nodeNum+1]);
          }
          break;
        }
      }
    }
  }
}

inline bool
AssemFlowGraph::_moveInstr(const vector<asm_arg *> & args,
    UIDMultiSetType & nodeUses, UIDMultiSetType & nodeDefs)
{
  const bool arg0_temp = _argIsTemp(args[0]);
  const bool arg1_temp = _argIsTemp(args[1]);

  bool isMove = arg0_temp && arg1_temp;

  if (arg0_temp) {
    asm_immediate_arg * arg = (asm_immediate_arg *) args[0];
    // Add to temps map
    tempsMap.putTemp( arg->content.regNum, true);
    // Add to uses
    nodeUses.insert( arg->content.regNum );
    switch(arg->type) {
      case REG_PRE_INCR:
      case REG_PRE_DECR:
      case REG_POST_INCR:
      case REG_POST_DECR:
      case ADDR_IN_REG_PRE_INCR:
      case ADDR_IN_REG_PRE_DECR:
      case ADDR_IN_REG_POST_INCR:
      case ADDR_IN_REG_POST_DECR:
        nodeDefs.insert( arg->content.regNum );
        isMove = false;
      default: break;
    }
  }
  if (arg1_temp) {
    asm_immediate_arg * arg = (asm_immediate_arg *) args[1];
    // Add to temps map
    tempsMap.putTemp( arg->content.regNum, true);
    // Add to defs
    nodeDefs.insert( arg->content.regNum );
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
  for(size_t nodeNum = 0; nodeNum < listOfNodes.size(); nodeNum++)
  {
    NodeType * const node = &listOfNodes[nodeNum];
    asm_statement * stmt = node->data;
    if (stmt->getType() == ASM_INSTRUCTION_STATEMENT) {
      asm_instruction_statement * i_stmt = (asm_instruction_statement *) stmt;

      UIDMultiSetType & nodeUses = uses[node];
      UIDMultiSetType & nodeDefs = defs[node];

      // Special treatment of "move" instruction
      if (i_stmt->instruction == MOV) {
        node->isMove = _moveInstr(i_stmt->args, nodeUses, nodeDefs);
      } else {
        // Other instructions
        for(size_t argNum = 0; argNum < i_stmt->args.size(); argNum++)
        {
          if ( _argIsTemp(i_stmt->args[argNum]) ) {
            asm_immediate_arg * arg = (asm_immediate_arg *)i_stmt->args[argNum];
            // to temps map
            tempsMap.putTemp(arg->content.regNum, true);
            // uses
            nodeUses.insert(arg->content.regNum);

            // defines
            if (_argIsDefined(i_stmt->instruction, argNum, arg->type)) {
              nodeDefs.insert(arg->content.regNum);
            }
          }
        } // End loop arguments
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

  this->printFlowGraph();

  DebugPrintf(("  - Adding Arcs\n"));
  // Assigns relations
  _createArcs(function.localSymbols);

  DebugPrintf(("  - Finding Uses and Defs\n"));
  // Finds uses and defines
  _findUsesDefines();
  DebugPrintf(("  - Done\n"));
}
