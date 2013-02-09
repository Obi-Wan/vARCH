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
  for(asm_function_param * obj : this->parameters) { delete obj; }
  for(asm_data_statement * obj : this->stackLocals) { delete obj; }
  for(asm_data_statement * obj : this->uniqueLocals) { delete obj; }
  for(asm_statement * obj : this->stmts) { delete obj; }

  for(ArgLabelRecord * ref : this->refs) { delete ref; }
}

/**
 * Adds locals. Needs to be ported to the new convention where the information
 * about const and shared is stored into the label and not into the referenced
 * object. Moreover, all the objects coming after a label will have its
 * properties, until they meet another label or the end.
 * @param locs
 */
void
asm_function::addLocals(list<asm_data_statement *> && locs)
{
  bool is_constant = false;
  bool is_shared = false;

  for(asm_data_statement * stmt : locs)
  {
    if (stmt->getType() == ASM_LABEL_STATEMENT)
    {
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
asm_function::addStmts(list<asm_statement *> && stmts)
{
  this->stmts.insert(this->stmts.end(), stmts.begin(), stmts.end());
}

void
asm_function::finalize()
{
  DebugPrintf(("- Adding stmts and locals to function: %s -\n", name.c_str()));

  for(asm_statement * stmt : stmts)
  {
    if (stmt->isInstruction())
    {
      asm_instruction_statement * istmt = (asm_instruction_statement *) stmt;

      for (asm_arg * arg : istmt->args)
      {
        if (arg->getType() == ASM_LABEL_ARG)
        {
          ArgLabelRecord * tempRecord = new ArgLabelRecord();
          tempRecord->arg = (asm_label_arg *) arg;
          tempRecord->parent = istmt;
          refs.push_back(tempRecord);
        }
      }
    } else {
      checkAndAddLabel(stmt);
    }
  }

  for(asm_data_statement * stmt : uniqueLocals)
  {
    checkAndAddLabel(stmt);
  }

  for(asm_data_statement * stmt : stackLocals)
  {
    checkAndAddLabel(stmt);
  }

  DebugPrintf(("- Terminated: Adding stmts and locals -\n\n"));
}

void
asm_function::rebuildOffsets()
{
  size_t tempLocalOffset = 0;
  for(asm_statement * stmt : stmts)
  {
    stmt->offset = tempLocalOffset;
    tempLocalOffset += stmt->getSize();
  }
  for(asm_data_statement * stmt : uniqueLocals)
  {
    stmt->offset = tempLocalOffset;
    tempLocalOffset += stmt->getSize();
  }

  // Distance from Frame Pointer
  size_t offsetFromFP = 0;
  const size_t allocatedSize = getStackedDataSize();

  for(asm_data_statement * stmt : stackLocals)
  {
    stmt->offset = allocatedSize - offsetFromFP - 4;
    offsetFromFP += stmt->getSize();
  }
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
