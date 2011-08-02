/*
 * AssemFlowGraph.cpp
 *
 *  Created on: 19/lug/2011
 *      Author: ben
 */

#include "AssemFlowGraph.h"

#include "exceptions.h"
#include "CpuDefinitions.h"

#include <sstream>

using namespace std;

inline bool
AssemFlowGraph::_argIsTemp(const asm_arg * const arg) const
{
  return (arg->getType() == ASM_IMMEDIATE_ARG
          && ((const asm_immediate_arg * const)arg)->isTemp);
}

inline bool
AssemFlowGraph::_argIsReg(const asm_arg * const arg) const
{
  return (arg->getType() == ASM_IMMEDIATE_ARG
          && ((const asm_immediate_arg * const)arg)->type != COST
          && ((const asm_immediate_arg * const)arg)->type != ADDR);
}

inline uint32_t
AssemFlowGraph::shiftArgUID(const asm_immediate_arg * arg, const bool & isTemp)
  const
{
  return isTemp * FIRST_TEMPORARY + 1 + arg->content.tempUID;
}

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
AssemFlowGraph::_generateMovesForFunctionCalls(asm_function & function)
{
  ListOfStmts & stmts = function.stmts;

  // Find return statements
  vector<ListOfStmts::iterator> returns;

  // Push temps map and fin max temp
  uint32_t minNewTemp = FIRST_TEMPORARY;

  for(ListOfStmts::iterator stmtIt = stmts.begin(); stmtIt != stmts.end();
      stmtIt++)
  {
    asm_statement * stmt = *stmtIt;
    if (stmt->isInstruction()) {
      asm_instruction_statement * i_stmt = (asm_instruction_statement *) stmt;
      vector<asm_arg *> args = i_stmt->args;
      for(size_t numArg = 0; numArg < args.size(); numArg++)
      {
        const bool isTemp = _argIsTemp(args[numArg]);
        if (isTemp || _argIsReg(args[numArg])) {
          asm_immediate_arg * arg = (asm_immediate_arg *) args[numArg];
          const uint32_t shiftedTempUID = shiftArgUID(arg, isTemp);
          // to temps map
          tempsMap.putTemp( shiftedTempUID, true);
          // find min avail temp
          if ( (shiftedTempUID+1) > minNewTemp) {
            minNewTemp = shiftedTempUID+1;
            DebugPrintf(("New minNewTemp %u\n", minNewTemp));
          }
        }
      }

      if (stmt->getType() == ASM_RETURN_STATEMENT) {
        DebugPrintf(("Found Return at: %d\n", stmt->position.first_line));
        returns.push_back(stmtIt);
      }
    }
  }

  // Take care of callee-save registers
  // (if it doesn't return, we don't even bother doing callee-save)
  if (returns.size()) {
    for(uint32_t regNum = STD_CALLEE_SAVE; regNum < NUM_REGS; regNum++)
    {
      const uint32_t tempProgr = minNewTemp++;
      tempsMap.putTemp( tempProgr, true);
      {
        const asm_statement * firstStmt = *function.stmts.begin();
        // Build the move temp[5-8] <- R[5-8]
        asm_instruction_statement * stmt =
            new asm_instruction_statement(firstStmt->position, MOV);

        asm_immediate_arg * regArg = new asm_immediate_arg(firstStmt->position);
        regArg->relative = false;
        regArg->type = REG;
        regArg->content.val = regNum;

        stmt->addArg(regArg);

        asm_immediate_arg * tempArg = new asm_immediate_arg(firstStmt->position);
        tempArg->relative = false;
        tempArg->type = REG;
        tempArg->content.val = tempProgr - FIRST_TEMPORARY - 1;
        tempArg->isTemp = true;

        stmt->addArg(tempArg);

        function.stmts.push_front(stmt);
      }
      for(vector<ListOfStmts::iterator>::iterator retIt = returns.begin();
          retIt != returns.end(); retIt++)
      {
        asm_return_statement * ret = (asm_return_statement *) **retIt;
        // Build the move R[5-8] <- temp[5-8]
        asm_instruction_statement * stmt =
            new asm_instruction_statement(ret->position, MOV);

        asm_immediate_arg * tempArg = new asm_immediate_arg(ret->position);
        tempArg->relative = false;
        tempArg->type = REG;
        tempArg->content.val = minNewTemp;
        tempArg->isTemp = true;

        stmt->addArg(tempArg);

        asm_immediate_arg * regArg = new asm_immediate_arg(ret->position);
        regArg->relative = false;
        regArg->type = REG;
        regArg->content.val = regNum;

        stmt->addArg(regArg);

        function.stmts.insert(*retIt, stmt);
      }
    }
  }

  // Push returns if needed
  for(vector<ListOfStmts::iterator>::iterator retIt = returns.begin();
      retIt != returns.end(); retIt++)
  {
    asm_return_statement * ret = (asm_return_statement *) **retIt;
    if (ret->args.size() == 1)
    {
      asm_arg * arg = ret->args[0];
      if (_argIsTemp(arg) || _argIsReg(arg))
      {
        asm_immediate_arg * i_arg = (asm_immediate_arg *) arg;

        // Build the move R1 <- tempRet
        asm_instruction_statement * stmt =
            new asm_instruction_statement(ret->position, MOV);

        asm_immediate_arg * tempArg = new asm_immediate_arg(ret->position);
        tempArg->relative = i_arg->relative;
        tempArg->type = i_arg->type;
        tempArg->content.val = i_arg->content.val;
        tempArg->isTemp = i_arg->isTemp;

        stmt->addArg(tempArg);

        asm_immediate_arg * regArg = new asm_immediate_arg(ret->position);
        regArg->relative = false;
        regArg->type = REG;
        regArg->content.val = 0;

        stmt->addArg(regArg);

        function.stmts.insert(*retIt, stmt);
      } else {
        throw WrongArgumentException(
            "Returned element should be a temporary or a register");
      }
    }
  }

  // Push moves of parameters in Function calls
//  for(ListOfStmts::iterator stmtIt = stmts.begin();
//      stmtIt != stmts.end(); stmtIt++)
//  {
//    asm_statement * stmt = *stmtIt;
//  }
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

inline bool
AssemFlowGraph::_moveInstr(const vector<asm_arg *> & args,
    UIDMultiSetType & nodeUses, UIDMultiSetType & nodeDefs)
{
  const bool arg0_temp = _argIsTemp(args[0]);
  const bool arg1_temp = _argIsTemp(args[1]);

  bool isMove = arg0_temp && arg1_temp;

  if (arg0_temp || _argIsReg(args[0])) {
    asm_immediate_arg * arg = (asm_immediate_arg *) args[0];
    const uint32_t shiftedTempUID = shiftArgUID(arg, arg0_temp);
    // Add to temps map
    tempsMap.putTemp( shiftedTempUID, true);
    // Add to uses
    nodeUses.insert( shiftedTempUID );
    // Test if it should be in Defines, too
    switch(arg->type) {
      case REG_PRE_INCR:
      case REG_PRE_DECR:
      case REG_POST_INCR:
      case REG_POST_DECR:
      case ADDR_IN_REG_PRE_INCR:
      case ADDR_IN_REG_PRE_DECR:
      case ADDR_IN_REG_POST_INCR:
      case ADDR_IN_REG_POST_DECR:
        nodeDefs.insert( shiftedTempUID );
        isMove = false;
      default: break;
    }
  }
  if (arg1_temp || _argIsReg(args[1])) {
    asm_immediate_arg * arg = (asm_immediate_arg *) args[1];
    const uint32_t shiftedTempUID = shiftArgUID(arg, arg1_temp);
    // Add to defs
    nodeDefs.insert( shiftedTempUID );
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
          const bool isTemp = _argIsTemp(i_stmt->args[argNum]);
          if ( isTemp || _argIsReg(i_stmt->args[argNum]) ) {
            asm_immediate_arg * arg = (asm_immediate_arg *)i_stmt->args[argNum];
            const uint32_t shiftedTempUID = shiftArgUID(arg, isTemp);
            // uses
            nodeUses.insert( shiftedTempUID );

            // defines
            if (_argIsDefined(i_stmt->instruction, argNum, arg->type)) {
              nodeDefs.insert( shiftedTempUID );
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
  _generateMovesForFunctionCalls(function);

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
        if (_argIsTemp(*argIt)) {
          asm_immediate_arg * arg = (asm_immediate_arg *) *argIt;
          const uint32_t temp_uid = shiftArgUID(arg, true);

          AssignedRegs::const_iterator reg = regs.find(temp_uid);
          if (reg == regs.end()) {
            throw WrongArgumentException(
                "A temporary in instruction was not considered! ("
                + tempsMap.getLabel(temp_uid) + ")");
          }
          if (reg->second) {
            arg->content.tempUID = reg->second -1;
          } else {
            throw WrongArgumentException(
                "Pending Spills! not using a solution!");
          }
        }
      }
    }
  }
}



