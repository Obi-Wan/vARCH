/* 
 * File:   asm_function.cpp
 * Author: ben
 * 
 * Created on 5 agosto 2010, 21.13
 */

#include "asm-function.h"

#include "exceptions.h"

asm_function::~asm_function()
{
  DEALLOC_ELEMS_VECTOR(this->parameters, ListOfParams);
  DEALLOC_ELEMS_VECTOR(this->stackLocals, ListOfDataStmts);
  DEALLOC_ELEMS_VECTOR(this->uniqueLocals, ListOfDataStmts);
  DEALLOC_ELEMS_VECTOR(this->stmts, ListOfStmts);
}

/**
 * Adds locals. Needs to be ported to the new convention where the information
 * about const and shared is stored into the label and not into the referenced
 * object. Moreover, all the objects coming after a label will have its
 * properties, until they meet another label or the end.
 * @param locs
 */
void
asm_function::addLocals(list<asm_data_statement *> * locs)
{
  bool is_constant = false;
  bool is_shared = false;

  for(list<asm_data_statement *>::iterator stmtIt = locs->begin();
      stmtIt != locs->end(); stmtIt++)
  {
    asm_data_statement * stmt = *stmtIt;
    if (stmt->getType() == ASM_LABEL_STATEMENT) {
      is_constant = stmt->is_constant;
      is_shared = stmt->is_shared;
    }

    if (is_constant || is_shared) {
      uniqueLocals.push_back( stmt );
    } else {
      stackLocals.push_back( stmt );
    }
    stmt->is_constant = is_constant;
    stmt->is_shared = is_shared;
  }
}

void
asm_function::addStmts(list<asm_statement *> * stmts)
{
  this->stmts.insert(this->stmts.end(), stmts->begin(), stmts->end());
}

void
asm_function::finalize()
{
  DebugPrintf(("- Adding stmts and locals to function: %s -\n", name.c_str()));
  size_t tempLocalOffset = 0;

  for(ListOfStmts::iterator stmt_it = stmts.begin(); stmt_it != stmts.end();
      stmt_it++)
  {
    asm_statement * stmt = *stmt_it;

    stmt->offset = tempLocalOffset;
    tempLocalOffset += stmt->getSize();

    if (stmt->isInstruction()) {
      asm_instruction_statement * istmt = (asm_instruction_statement *) stmt;
      for (size_t argNum = 0; argNum < istmt->args.size(); argNum++) {
        if (istmt->args[argNum]->getType() == ASM_LABEL_ARG) {
          ArgLabelRecord * tempRecord = new ArgLabelRecord();
          tempRecord->arg = (asm_label_arg *)istmt->args[argNum];
          tempRecord->parent = istmt;
          refs.push_back(tempRecord);
        }
      }
    } else {
      checkAndAddLabel(stmt);
    }
  }

  for(size_t index = 0; index < uniqueLocals.size(); index++) {
    asm_data_statement * stmt = uniqueLocals[index];

    stmt->offset = tempLocalOffset;
    tempLocalOffset += stmt->getSize();
    checkAndAddLabel(stmt);
  }

  // Distance from stack pointer
  size_t offsetFromStackPtr = 0;
  const size_t allocatedSize = getStackedDataSize();
  for(size_t index = 0; index < stackLocals.size(); index++) {
    asm_data_statement * stmt = stackLocals[index];

    stmt->offset = allocatedSize - offsetFromStackPtr;
    offsetFromStackPtr += stmt->getSize();
    checkAndAddLabel(stmt);
  }

  DebugPrintf(("- Terminated: Adding stmts and locals -\n\n"));
}

void
asm_function::rebuildOffsets()
{
  size_t tempLocalOffset = 0;
  for(ListOfStmts::iterator stmt_it = stmts.begin(); stmt_it != stmts.end();
      stmt_it++)
  {
    asm_statement * stmt = *stmt_it;

    stmt->offset = tempLocalOffset;
    tempLocalOffset += stmt->getSize();
  }
  for(size_t i = 0; i < uniqueLocals.size(); i++) {
    asm_data_statement * stmt = uniqueLocals[i];

    stmt->offset = tempLocalOffset;
    tempLocalOffset += stmt->getSize();
  }

  // Distance from stack pointer
  size_t offsetFromStackPtr = 0;
  const size_t allocatedSize = getStackedDataSize();
  for(size_t i = 0; i < stackLocals.size(); i++) {
    asm_data_statement * stmt = stackLocals[i];

    stmt->offset = allocatedSize - offsetFromStackPtr;
    offsetFromStackPtr += stmt->getSize();
  }
}

bool
asm_function::ensureTempsUsage(const bool & used) const
{
  bool error = false;
  for(const asm_statement * stmt : stmts)
  {
    if (stmt->getType() == ASM_INSTRUCTION_STATEMENT) {
      try {
        ((const asm_instruction_statement *)stmt)->ensureTempsUsage(used);
      } catch (const WrongArgumentException & e) {
        fprintf(stderr, "ERROR: in instruction!\n%s\n", e.what());
        error = true;
      }
    }
  }
  return error;
}

inline void
asm_function::checkAndAddLabel(asm_statement * stmt)
{
  if (stmt->getType() == ASM_LABEL_STATEMENT) {
    asm_label_statement * l_stmt = (asm_label_statement *) stmt;
    DebugPrintf(("Found local label: %s in function %s!\n",
                 l_stmt->label.c_str(), this->name.c_str()));
    localSymbols.addLabel(l_stmt);
  }
}
