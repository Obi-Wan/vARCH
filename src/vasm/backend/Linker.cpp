/*
 * Linker.cpp
 *
 *  Created on: 09/feb/2013
 *      Author: ben
 */

#include "Linker.h"

#include "ErrorReporter.h"

void
Linker::prelink()
{
  this->rebuildOffsets(true);
  this->exposeGlobalLabels();
  this->assignValuesToLocalLabels();
}

void
Linker::link()
{
  this->checkNoPrototypes();

  this->moveMainToTop();
  this->rebuildOffsets();
  this->updateFunctionLabels();
  this->assignValuesToLabels();
}

INLINE void
Linker::checkNoPrototypes() const
{
  ErrorReporter errOut;

  for(const asm_function * func : program.functions)
  {
    if (func->stmts.empty()) {
      errOut.addErrorMsg(func->position,
          "Undefined reference to function: " + func->name);
    }
  }
  if (errOut.hasErrors()) {
    errOut.throwError( WrongArgumentException("Errors in linking") );
  }
}

INLINE void
Linker::moveMainToTop()
{
  /* Let's put the main function in front of all the others */
  for(std::deque<asm_function *>::iterator iter = program.functions.begin();
      iter != program.functions.end(); iter++)
  {
    if (!(*iter)->name.compare("main")) {
      /* Main found! */
      asm_function * main = (*iter);
      program.functions.erase(iter);
      program.functions.insert(program.functions.begin(), main);
      break;
    }
  }
}

INLINE void
Linker::rebuildOffsets(const bool & is_obj)
{
  size_t tempOffset = 0;
  for(asm_function * const func : program.functions)
  {
    func->functionOffset = tempOffset;
    rebuildFunctionOffsets(*func);

    tempOffset += func->getSize();
  }
  if (is_obj)
  {
    tempOffset = 0;
  }
  for(asm_data_statement * const stmt : program.shared_vars)
  {
    stmt->offset = tempOffset;
    tempOffset += stmt->getSize();
  }
  if (is_obj)
  {
    tempOffset = 0;
  }
  for(asm_data_statement * const stmt : program.constants)
  {
    stmt->offset = tempOffset;
    tempOffset += stmt->getSize();
  }
}

INLINE void
Linker::rebuildFunctionOffsets(asm_function & func)
{
  size_t tempLocalOffset = 0;
  for(asm_statement * stmt : func.stmts)
  {
    stmt->offset = tempLocalOffset;
    tempLocalOffset += stmt->getSize();
  }

  // Distance from Frame Pointer
  size_t offsetFromFP = 0;
  const size_t allocatedSize = func.getStackedDataSize();

  for(asm_data_statement * stmt : func.stackLocals)
  {
    stmt->offset = allocatedSize - offsetFromFP - 4;
    offsetFromFP += stmt->getSize();
  }
}

INLINE void
Linker::exposeGlobalLabels()
{
  ErrorReporter errOut;

  /* And fix their labels */
  DebugPrintf(("-- Adding Functions to Global Labels - Phase --\n"));

  for(asm_function * const func : program.functions)
  {
    asm_label_statement * const tempLabel =
        new asm_label_statement(func->position, func->name, func->getSize(), 1, true, true);
    tempLabel->offset = func->functionOffset;
    func->stmts.push_front(tempLabel);

    try {
      program.globalSymbols.addLabel(tempLabel);
    } catch (const WrongArgumentException & e) {
      errOut.addErrorMsg(tempLabel->position, e);
    }
  }
  for(asm_data_statement * const stmt : program.shared_vars)
  {
    try {
      if (stmt->getType() == ASM_LABEL_STATEMENT) {
        asm_label_statement * l_stmt = (asm_label_statement *) stmt;
        DebugPrintf(("Found label statement: %s!\n", l_stmt->label.c_str() ));

        program.globalSymbols.addLabel(l_stmt);
      }
    } catch (const WrongArgumentException & e) {
      errOut.addErrorMsg(stmt->position, e);
    }
  }
  for(asm_data_statement * const stmt : program.constants)
  {
    try {
      if (stmt->getType() == ASM_LABEL_STATEMENT) {
        asm_label_statement * l_stmt = (asm_label_statement *) stmt;
        DebugPrintf(("Found label statement: %s!\n", l_stmt->label.c_str() ));

        program.globalSymbols.addLabel(l_stmt);
      }
    } catch (const WrongArgumentException & e) {
      errOut.addErrorMsg(stmt->position, e);
    }
  }
  DebugPrintf(("-- Terminated: Adding Funcs to Global Labels - Phase --\n\n"));
  if (errOut.hasErrors()) {
    errOut.throwError( WrongArgumentException("Errors in labels definition") );
  }
}

INLINE void
Linker::updateFunctionLabels()
{
  ErrorReporter errOut;

  for(asm_function * const func : program.functions)
  {
    try {
      asm_label_statement * label = program.globalSymbols.getStmt(func->name);
      label->offset = func->functionOffset;
    } catch (const WrongArgumentException & e) {
      errOut.addErrorMsg(func->position, e);
    }
  }
  if (errOut.hasErrors()) {
    errOut.throwError( WrongArgumentException("Errors in function labels update") );
  }
}

INLINE void
Linker::assignValuesToLocalLabels()
{
  bool error = false;

  DebugPrintf(("-- Assign Local labels - Phase --\n"));

  for(asm_function * func : program.functions)
  {
    DebugPrintf((" - Processing references of function: %s\n",
                  func->name.c_str()));
    for(ArgLabelRecord * ref : func->refs)
    {
      asm_label_arg & argument = *(ref->arg);
      const std::string & labelName = argument.label;

      DebugPrintf(("  - Processing label: %s\n", labelName.c_str()));
      asm_label_statement * localLabel = func->localSymbols.getStmt(labelName);

      if (localLabel)
      {
        DebugPrintf(("    It is local, with position (in the program): %3u\n"
                      "      is const: %5s, is shared: %5s\n",
                      (uint32_t)localLabel->offset,
                      localLabel->is_constant ? "true" : "false",
                      localLabel->is_shared ? "true" : "false"));

        if (localLabel->isConst())
        {
          argument.type = ref->arg->type;
          if (argument.type == IMMED && ref->parent->isInstruction()
              && (((asm_instruction_statement *)ref->parent)->instruction & JUMP) == JUMP)
          {
            argument.content.val = int64_t(localLabel->offset)
                  - int64_t(ref->parent->getSize()) - int64_t(ref->parent->offset);
          }
          else
          {
            // Should never happen ?
            argument.content.tempUID = uint32_t(localLabel->offset);
          }
        }
        else
        {
          argument.type = DISPLACED;
          if (args.getOmitFramePointer()) {
            argument.content.regNum = STACK_POINTER;
            argument.displacement = (int32_t)(func->getStackedDataSize() - localLabel->offset);
          } else {
            argument.content.regNum = FRAME_POINTER;
            argument.displacement = - int32_t(localLabel->offset);
          }
        }
      }
    }
  }
  DebugPrintf(("-- Terminated: Assign labels - Phase --\n\n"));
  if (error) {
    throw WrongArgumentException("Errors in labels assignment");
  }
}

INLINE void
Linker::assignValuesToLabels()
{
  bool error = false;

  DebugPrintf(("-- Assign labels - Phase --\n"));

  for(asm_function * func : program.functions)
  {
    DebugPrintf((" - Processing references of function: %s\n",
                  func->name.c_str()));
    for(ArgLabelRecord * ref : func->refs)
    {
      asm_label_arg & argument = *(ref->arg);
      const std::string & labelName = argument.label;

      DebugPrintf(("  - Processing label: %s\n", labelName.c_str()));
      asm_label_statement * localLabel = func->localSymbols.getStmt(labelName);

      // Let's check for local static variables: "label" -> "function_name::label"
      asm_label_statement * localStaticLabel =
          program.globalSymbols.getStmt(labelName, func->name);

      asm_label_statement * globalLabel = program.globalSymbols.getStmt(labelName);

      if (localLabel)
      {
        DebugPrintf(("    It is local, with position (in the program): %3u\n"
                      "      is const: %5s, is shared: %5s\n",
                      (uint32_t)localLabel->offset,
                      localLabel->is_constant ? "true" : "false",
                      localLabel->is_shared ? "true" : "false"));

        if (localLabel->isConst())
        {
//          // FIXME: should be changed to DISPLACED
//          argument.content.val =
//              int64_t(localLabel->offset + func->functionOffset);
          // This now only happens for code labels (loops, if-else, jumps)
//          argument.type = DISPLACED;
//          argument.content.regNum = PROGRAM_COUNTER;
          argument.type = ref->arg->type;
          if (argument.type == IMMED && ref->parent->isInstruction()
              && (((asm_instruction_statement *)ref->parent)->instruction & JUMP) == JUMP)
          {
            argument.content.val = int64_t(localLabel->offset)
                  - int64_t(ref->parent->getSize()) - int64_t(ref->parent->offset);
          }
          else
          {
            argument.content.tempUID = uint32_t(localLabel->offset);
          }
        }
        else
        {
          argument.type = DISPLACED;
          if (args.getOmitFramePointer()) {
            argument.content.regNum = STACK_POINTER;
            argument.displacement = (int32_t)(func->getStackedDataSize() - localLabel->offset);
          } else {
            argument.content.regNum = FRAME_POINTER;
            argument.displacement = - int32_t(localLabel->offset);
          }
        }
      }
      else if (localStaticLabel)
      {
        // Local static variables will be in global, with a prefix:
        // "label" -> "function_name::label"
        DebugPrintf(("    It is local static (global with prefix), with position: %3u\n",
                    (uint32_t)localStaticLabel->offset));
        argument.type = ref->arg->type;
        if (argument.type == IMMED && ref->parent->isInstruction()
            && (((asm_instruction_statement *)ref->parent)->instruction & JUMP) == JUMP)
        {
          argument.content.val = int64_t(localStaticLabel->offset)
                - int64_t(ref->parent->getSize()) - int64_t(ref->parent->offset);
        }
        else
        {
          argument.content.tempUID = (uint32_t)localStaticLabel->offset;
        }
      }
      else if (globalLabel)
      {
        DebugPrintf(("    It is global, with position: %3u\n",
                    (uint32_t)globalLabel->offset));
        argument.type = ref->arg->type;
        if (argument.type == IMMED && ref->parent->isInstruction()
            && (((asm_instruction_statement *)ref->parent)->instruction & JUMP) == JUMP)
        {
          argument.content.val = int64_t(globalLabel->offset)
                - int64_t(ref->parent->getSize()) - int64_t(ref->parent->offset) - int64_t(func->functionOffset);
        }
        else
        {
          argument.content.tempUID = (uint32_t)globalLabel->offset;
        }
      }
      else
      {
        DebugPrintf(("      ERROR!! It is not even global!\n"));
        fprintf(stderr, "ERROR:%s at Line %4d\n%s\n"
                "--> Reference to not existing Label: '%s'\n",
                argument.position.fileNode->printString().c_str(),
                argument.position.first_line,
                argument.position.fileNode->printStringStackIncludes().c_str(),
                labelName.c_str());
        error = true;
      }
    }
  }
  DebugPrintf(("-- Terminated: Assign labels - Phase --\n\n"));
  if (error) {
    throw WrongArgumentException("Errors in labels assignment");
  }
}
