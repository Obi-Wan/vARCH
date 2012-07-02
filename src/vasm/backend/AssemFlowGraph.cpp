/*
 * AssemFlowGraph.cpp
 *
 *  Created on: 19/lug/2011
 *      Author: ben
 */

#include "AssemFlowGraph.h"

#include "exceptions.h"
#include "CpuDefinitions.h"
#include "Frame.h"

#include "../IncludesTree.h"

#include <sstream>
#include <algorithm>

using namespace std;

/**
 * Algorithmic class, which finds and stores uninitialized temporaries
 */
class VerifyUninitialized {
  vector<uint32_t> & uninit;
public:
  VerifyUninitialized(vector<uint32_t> & ui) : uninit(ui) { }

  void operator() (const uint32_t & temp) {
    if (temp >= (FIRST_TEMPORARY+1)) {
      uninit.push_back(temp);
    }
  }
};

/**
 * Algorithmic class, that finds where uninitialized temporaries were used, and
 * prints it a given stream
 */
class ReportUninitialized {
  stringstream & stream;
  const TempsMap & tm;
  const AssemFlowGraph & fg;
public:
  ReportUninitialized(stringstream & _ss, const TempsMap & _tm,
      const AssemFlowGraph & _fg)
    : stream(_ss), tm(_tm), fg(_fg)
  { }

  void operator() (const uint32_t & temp) {
    stream << "  Temporary: " << tm.getLabel(temp)
              << " was used uninitialized at:" << endl;
    for(AssemFlowGraph::nl_c_iterator nodeIt = fg.getListOfNodes().begin();
        nodeIt != fg.getListOfNodes().end(); nodeIt++)
    {
      const AssemFlowGraph::NodeType * const node = &*nodeIt;
      const AssemFlowGraph::UIDMultiSetType & nodeUses =
                                                fg.getUses().find(node)->second;
      const AssemFlowGraph::us_c_iterator useIt = nodeUses.find(temp);
      if (useIt != nodeUses.end()) {
        stream << "  - " << node->data->position.fileNode->printString()
              << " Line: " << node->data->position.first_line << ".\n"
              << node->data->position.fileNode->printStringStackIncludes()
              << endl;
      }
      // If then it is defined, let's stop searching
      const AssemFlowGraph::UIDMultiSetType & nodeDefs =
                                                fg.getDefs().find(node)->second;
      const AssemFlowGraph::us_c_iterator defIt = nodeDefs.find(temp);
      if (defIt != nodeDefs.end()) break;
    }
  }
};

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

    const NodeType * const node = &getListOfNodes().back();
    backReference.insert(StmtToNode::value_type(stmt, node));
  }
}

inline void
AssemFlowGraph::_createArcs(const TableOfSymbols & functionSymbols)
{
  for(nl_c_iterator nodeIt = getListOfNodes().begin();
      nodeIt != getListOfNodes().end(); nodeIt++)
  {
    const NodeType * const node = &*nodeIt;
    nl_c_iterator nextNodeIt = nodeIt;
    nextNodeIt++;

    asm_statement * stmt = node->data;
    if (stmt->getType() == ASM_LABEL_STATEMENT) {
      if (nextNodeIt != getListOfNodes().end()) {
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
          if (nextNodeIt != getListOfNodes().end()) {
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
          if (nextNodeIt != getListOfNodes().end()) {
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
  if (!Frame::shiftedIsSpecialReg(shiftedUID))
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

  const bool arg0_reg = args[0]->isReg();
  const bool arg1_reg = args[1]->isReg();

  bool isMove = (arg0_reg && arg1_reg) && (arg0_temp || arg1_temp);

  if (arg0_temp || arg0_reg) {
    asm_immediate_arg * arg = (asm_immediate_arg *) args[0];
    const uint32_t shiftedTempUID = Frame::shiftArgUID(arg, arg0_temp);
//      // Add to temps map
//      tempsMap.putTemp( shiftedTempUID, true);
    // Add to uses
    _addToSet(nodeUses, shiftedTempUID );
    // Test if it should be in Defines, too
    switch(arg->regModType) {
      case REG_PRE_INCR:
      case REG_PRE_DECR:
      case REG_POST_INCR:
      case REG_POST_DECR: {
        if ((arg->type & INDEXED) == INDEXED) {
          const uint32_t shiftedTempIndexUID = Frame::shiftArgUID(arg->index, arg->isIndexTemp);
          _addToSet(nodeDefs, shiftedTempIndexUID );
        } else {
          _addToSet(nodeDefs, shiftedTempUID );
        }
        isMove = false;
        break;
      }
      default: break;
    }
  }
  if (arg1_temp || arg1_reg) {
    asm_immediate_arg * arg = (asm_immediate_arg *) args[1];
    const uint32_t shiftedTempUID = Frame::shiftArgUID(arg, arg1_temp);
    switch (arg->type) {
      case REG_INDIR:
      case MEM_INDIR:
      case DISPLACED: {
        // Add to uses
        _addToSet(nodeUses, shiftedTempUID );
        isMove = false;
        switch (arg->regModType) {
          case REG_PRE_INCR:
          case REG_PRE_DECR:
          case REG_POST_INCR:
          case REG_POST_DECR:
            // Add to defs
            _addToSet(nodeDefs, shiftedTempUID );
          default: break;
        }
        break;
      }
      case INDEXED:
      case INDX_DISP: { // regModType in this case refers to the index
        // Add to uses
        _addToSet(nodeUses, shiftedTempUID );
        isMove = false;
        switch (arg->regModType) {
          case REG_PRE_INCR:
          case REG_PRE_DECR:
          case REG_POST_INCR:
          case REG_POST_DECR: {
            const uint32_t shiftedTempIndexUID = Frame::shiftArgUID(arg->index, arg->isIndexTemp);
            // Add to defs
            _addToSet(nodeDefs, shiftedTempIndexUID );
          }
          default: break;
        }
        break;
      }
      default: {
        // Add to defs
        _addToSet(nodeDefs, shiftedTempUID );
        break;
      }
    }
  }
  return isMove;
}

inline bool
AssemFlowGraph::_argIsDefined(const int & instruction, const size_t & argNum,
    const TypeOfArgument & argType, const ModifierOfArgument & argMod)
  const
{
  switch(argType) {
    case IMMED:
    case DIRECT: {
      return false;
    }
    case REG: {
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
        default: break;
      }
      break;
    }
    default: break;
  }
  return (IS_PRE_POST_MOD(argMod));
}

inline void
AssemFlowGraph::_findUsesDefines()
{
  for(nl_c_iterator nodeIt = getListOfNodes().begin();
      nodeIt != getListOfNodes().end(); nodeIt++)
  {
    /* Explicit override of default const behavior */
    NodeType * const node = (NodeType *) &*nodeIt;

    asm_statement * stmt = node->data;
    if (stmt->isInstruction()) {
      asm_instruction_statement * i_stmt = (asm_instruction_statement *) stmt;

      UIDMultiSetType & nodeUses = uses[node];
      UIDMultiSetType & nodeDefs = defs[node];

      if (stmt->getType() == ASM_INSTRUCTION_STATEMENT) {
        // Special treatment of "move" instruction
        if (i_stmt->instruction == MOV) {
          node->isMove = _moveInstr(i_stmt->args, nodeUses, nodeDefs);
          DebugPrintf(("Considering Move instr at line %d: is %s\n",
              i_stmt->position.first_line, node->isMove ? "Move" : "NOT Move"));
        } else {
          // Other instructions
          for(size_t argNum = 0; argNum < i_stmt->args.size(); argNum++)
          {
            const bool isTemp = i_stmt->args[argNum]->isTemporary();
            if ( isTemp || i_stmt->args[argNum]->isReg() ) {
              asm_immediate_arg * arg =
                                      (asm_immediate_arg *)i_stmt->args[argNum];
              const uint32_t shiftedTempUID =
                                        Frame::shiftArgUID(arg, isTemp);

              // uses
              _addToSet( nodeUses, shiftedTempUID );

              // defines
              if (_argIsDefined(i_stmt->instruction, argNum, arg->type, arg->regModType)) {
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
                                      Frame::shiftArgUID(numReg, false);
          _addToSet( nodeDefs, shiftedTempUID );
        }
        // It uses the arguments
        for(ConstListOfParams::const_iterator parIt = f_stmt->parameters.begin();
            parIt != f_stmt->parameters.end(); parIt++)
        {
          const asm_function_param * par = *parIt;
          const asm_immediate_arg * source =
                                        (const asm_immediate_arg *)par->source;
          const uint32_t shiftedTempUID =
                              Frame::shiftArgUID(source->content.regNum, false);
          _addToSet( nodeUses, shiftedTempUID );
        }
      } else if (stmt->getType() == ASM_RETURN_STATEMENT) {
        // Probably it uses the callee-save registers, and returned element
        for(uint32_t regNum = STD_CALLEE_SAVE; regNum < NUM_REGS; regNum++)
        {
          const uint32_t shiftedTempUID =
                                      Frame::shiftArgUID(regNum, false);
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
AssemFlowGraph::checkTempsUsedUndefined(const asm_function & func,
    const LiveMap<asm_statement *> & lm)
  const
{
  CHECK_THROW( (getListOfNodes().begin() != getListOfNodes().end()),
      WrongArgumentException("Not possible to check for temporaries used, while"
                        " undefined, because an empty function was passed.") );
  const NodeType * const node = &*getListOfNodes().begin();

  const LiveMap<asm_statement*>::um_c_iterator liveIns = lm.liveIn.find(node);
  CHECK_THROW( liveIns != lm.liveIn.end(),
      WrongArgumentException("Live map doesn't correspond to the function") );

  const LiveMap<asm_statement *>::UIDSetType & usedUndef = liveIns->second;
  /// The unwanted

  vector<uint32_t> uninit;
  VerifyUninitialized  verifier(uninit);
  for_each(usedUndef.begin(), usedUndef.end(), verifier );

  if (!uninit.empty()) {
    stringstream stream;

    ReportUninitialized reporter(stream, tempsMap, *this);

    stream << "Found temporaries used without being initialized, in "
            << "function: " << func.name << endl;
    for_each(uninit.begin(), uninit.end(), reporter);

    throw WrongArgumentException(stream.str());
  }
}

void
AssemFlowGraph::applySelectedRegisters(const AssignedRegs & regs)
{
  for(nl_c_iterator nodeIt = getListOfNodes().begin();
      nodeIt != getListOfNodes().end(); nodeIt++)
  {
    if (nodeIt->data->getType() == ASM_INSTRUCTION_STATEMENT) {
      asm_instruction_statement * stmt =
                                    (asm_instruction_statement *) nodeIt->data;
      for(vector<asm_arg *>::iterator argIt = stmt->args.begin();
          argIt != stmt->args.end(); argIt++)
      {
        if ((*argIt)->isTemporary()) {
          asm_immediate_arg * arg = (asm_immediate_arg *) *argIt;
          const uint32_t temp_uid = Frame::shiftArgUID(arg, true);

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

void
AssemFlowGraph::applySelectedRegisters(const AssignedRegs & regs,
    const AliasMap & aliases)
{
  for(nl_c_iterator nodeIt = getListOfNodes().begin();
      nodeIt != getListOfNodes().end(); nodeIt++)
  {
    if (nodeIt->data->getType() == ASM_INSTRUCTION_STATEMENT) {
      asm_instruction_statement * stmt =
                                    (asm_instruction_statement *) nodeIt->data;
      for(vector<asm_arg *>::iterator argIt = stmt->args.begin();
          argIt != stmt->args.end(); argIt++)
      {
        if ((*argIt)->isTemporary()) {
          asm_immediate_arg * arg = (asm_immediate_arg *) *argIt;
          const uint32_t temp_uid = Frame::shiftArgUID(arg, true);

          /* Verify that maybe it is an alias */
          const uint32_t temp_alias = aliases.getFinal(temp_uid);

          uint32_t to_be_assigned = 0;
          if (temp_alias && temp_alias < (FIRST_TEMPORARY + 1) ) {
            to_be_assigned = temp_alias;
          } else {
            /* temp_alias is != 0 then we should use the aliased value */
            AssignedRegs::const_iterator reg
                                = regs.find(temp_alias ? temp_alias : temp_uid);
            if (reg == regs.end()) {
              throw WrongArgumentException(
                  "A temporary in instruction was not considered! ("
                  + tempsMap.getLabel(temp_uid) + ")");
            }
            to_be_assigned = reg->second;
          }
          if (to_be_assigned) {
            arg->content.tempUID = to_be_assigned -1;
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



