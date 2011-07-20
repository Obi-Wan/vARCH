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
  stream << stmt->toString();
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
    if (stmt->getType() == ASM_INSTRUCTION_STATEMENT) {
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
            addDirectedArc(node, &listOfNodes[nodeNum+1]);
          }
          break;
        }
      }
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

      int instruction = i_stmt->instruction;

      for(size_t argNum = 0; argNum < i_stmt->args.size(); argNum++)
      {
        asm_arg * argIt = i_stmt->args[argNum];
//        switch(GET_ARG(argNum, i_stmt->instruction)) {
//          case ADDR_IN_REG:
//          case REG_PRE_INCR:
//          case REG_PRE_DECR:
//          case REG_POST_INCR:
//          case REG_POST_DECR:
//          case ADDR_IN_REG_PRE_INCR:
//          case ADDR_IN_REG_PRE_DECR:
//          case ADDR_IN_REG_POST_INCR:
//          case ADDR_IN_REG_POST_DECR:
//
//        }
        if ( argIt->getType() == ASM_IMMEDIATE_ARG )
        {
          asm_immediate_arg * arg = (asm_immediate_arg *) argIt;
          if (arg->isTemp) {
            // to temps map
            tempsMap.putTemp(arg->content.regNum);

            // uses
            uses[node].insert(arg->content.regNum);

            // defines
            if (argNum == 0) {
              switch (instruction) {
                case NOT:
                case INCR:
                case DECR:
                case COMP2:
                case LSH:
                case RSH:
                  defs[node].insert(arg->content.regNum);
                  break;
                default:
                  break;
              }
            } else {
              switch (instruction) {
                case MOV:
                case ADD:
                case MULT:
                case SUB:
                case DIV:
                case QUOT:
                case AND:
                case OR:
                case XOR:
                case GET:
                  defs[node].insert(arg->content.regNum);
                  break;
                default:
                  break;
              }
            }
          }
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
  // import nodes from assembler code
  _addNodesToGraph(function);

  // Assigns relations
  _createArcs(function.localSymbols);

  // Finds uses and defines
  _findUsesDefines();
}
