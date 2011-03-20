/* 
 * File:   asm_program.cpp
 * Author: ben
 * 
 * Created on 5 agosto 2010, 21.05
 */

#include "asm-program.h"

asm_program::asm_program(list<asm_function *> * _funcs,
              list<asm_data_statement *> * _globals) : tempOffset(0)
{
  /* Let's put the main function in front of all the others */
  for(list<asm_function *>::iterator iter = _funcs->begin();
      iter != _funcs->end(); iter++)
  {
    if (!(*iter)->name.compare("main")) {
      /* Main found! */
      asm_function * main = (*iter);
      _funcs->erase(iter);
      _funcs->insert(_funcs->begin(), main);
      break;
    }
  }
  /* Now let's copy them, and the data */
  functions.reserve(_funcs->size());
  functions.insert(functions.begin(), _funcs->begin(), _funcs->end());
  globals.reserve(_globals->size());
  globals.insert(globals.begin(), _globals->begin(), _globals->end());

  /* And fix their labels */
  DebugPrintf(("-- Adding Functions to Global Labels - Phase --\n"));
  for(size_t index = 0; index < functions.size(); index++) {
    asm_function * func = functions[index];

    asm_label_statement * tempLabel =
        new asm_label_statement(func->position, func->name);
    tempLabel->offset = tempOffset;

    globalSymbols.addLabel(tempLabel);

    tempOffset += func->getSize();
  }
  for(size_t index = 0; index < globals.size(); index++) {
    asm_data_statement * stmt = globals[index];

    stmt->offset = tempOffset;
    tempOffset += stmt->getSize();
    if (stmt->getType() == ASM_LABEL_STATEMENT) {
      DebugPrintf(("Found label statement: %s!\n",
             ((asm_label_statement *)stmt)->label.c_str()));
      globalSymbols.addLabel((asm_label_statement *)stmt);
    }
  }
  DebugPrintf(("-- Terminated: Adding Funcs to Global Labels - Phase --\n\n"));
}

void asm_program::assignValuesToLabels() {
  DebugPrintf(("-- Assign labels - Phase --\n"));
  for(size_t index = 0; index < functions.size(); index++)
  {
    DebugPrintf((" - Processing references of function: %s\n",
                  functions[index]->name.c_str()));
    for(list<argLabelRecord *>::iterator ref  = functions[index]->refs.begin();
            ref != functions[index]->refs.end(); ref++)
    {
      asm_label_arg & argument = *((*ref)->arg);
      const string & labelName = argument.label;

      DebugPrintf(("  - Processing label: %s\n", labelName.c_str()));
      int pos = functions[index]->localSymbols.getPositionOfLabel(labelName);

      if (pos < 0) {
        DebugPrintf(("    It's not local since it returned: %3d\n", pos));
        pos = globalSymbols.getPositionOfLabel(labelName);
        if (pos < 0) {
          DebugPrintf(("    It's not even global (returned %3d)! ERROR!!\n",
                        pos));
          throw WrongArgumentException("reference to a Label called: " +
                  labelName + " that does not exist");
        } else {
          DebugPrintf(("    It's global and at: %3d\n", pos));

          argument.pointedPosition = pos;
          argument.relative = false;
        }
      } else {
        DebugPrintf(("    It's local since it returned: %3d\n", pos));
        DebugPrintf(("    Calculating sizes:\n"));
        DebugPrintf(("      position of stmt: %03d\n", (*ref)->parent->offset));
        DebugPrintf(("      size of statement and args: %03d\n",
                      (*ref)->parent->getSize()));
        DebugPrintf(("      position of the arg relative to the stmt: %03d\n",
                      argument.relOffset));

        argument.pointedPosition =
            pos - argument.relOffset - (*ref)->parent->offset;
        argument.relative = true;
      }
    }
  }
  DebugPrintf(("-- Terminated: Assign labels - Phase --\n\n"));
}

void asm_program::assemble(const string & outputName) {
  Bloat bytecode;
  bytecode.resize(tempOffset,0);
  Bloat::iterator pos = bytecode.begin();
  for (size_t funcIndex = 0; funcIndex < functions.size(); funcIndex++) {
    asm_function & func = *functions[funcIndex];
    for(size_t j = 0; j < func.stmts.size(); j++) {
      func.stmts[j]->emitCode(pos);
    }
    for(size_t j = 0; j < func.locals.size(); j++) {
      func.locals[j]->emitCode(pos);
    }
  }
  for (size_t index = 0; index < globals.size(); index++) {
    globals[index]->emitCode(pos);
  }

  InfoPrintf(("Size of the generated file: %5u bytes in %4u cells\n",
              (uint32_t)bytecode.size()*4, (uint32_t)bytecode.size() ));
#ifdef DEBUG
  DebugPrintf(("-- Dumping generated binary code --\n"));
  for (uint64_t i = 0; i < bytecode.size(); i++) {
    DebugPrintf(("Mem %03lu: %12d\n", i, bytecode[i]));
  }
#endif

  BinWriter writer(outputName.c_str());
  writer.saveBinFileContent(bytecode);
}

