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
  /* Let's put the main function in front of all te others */
  for(list<asm_function *>::iterator iter = _funcs->begin();
          iter != _funcs->end(); iter++) {
    if (!(*iter)->name.compare("main")) {
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
  for(int i = 0; i < functions.size(); i++) {
    asm_function * func = functions[i];

    asm_label_statement * tempLabel =
        new asm_label_statement(func->position, func->name);
    tempLabel->offset = tempOffset;

    globalSymbols.addLabel(tempLabel);

    tempOffset += func->getSize();
  }
  for(int i = 0; i < globals.size(); i++) {
    asm_data_statement * stmt = globals[i];

    stmt->offset = tempOffset;
    tempOffset += stmt->getSize();
    if (stmt->getType() == ASM_LABEL_STATEMENT) {
      DebugPrintf(("Found label statement: %s!\n",
             ((asm_label_statement *)stmt)->label.c_str()));
      globalSymbols.addLabel((asm_label_statement *)stmt);
    }
  }
}

void asm_program::assignValuesToLabels() {
  DebugPrintf(("Assigning labels:\n"));
  for(int i = 0; i < functions.size(); i++) {

    DebugPrintf(("Processing refs of function: %s\n",functions[i]->name.c_str()));
    for(list<argLabelRecord *>::iterator j  = functions[i]->refs.begin();
            j != functions[i]->refs.end(); j++) {
      DebugPrintf(("Processing label: %s\n", (*j)->arg->label.c_str()));
      int pos = functions[i]->localSymbols.getPositionOfLabel((*j)->arg->label);
      if (pos < 0) {
        DebugPrintf(("It's not local since it returned: %3d\n", pos));
        pos = globalSymbols.getPositionOfLabel((*j)->arg->label);
        if (pos < 0) {
          DebugPrintf(("It's not even global (returned %3d)! ERROR!!\n", pos));
          throw WrongArgumentException("reference to a Label called: " +
                  (*j)->arg->label + " that does not exist");
        } else {
          DebugPrintf(("It's global and at: %3d\n", pos));
          (*j)->arg->pointedPosition = pos;
          (*j)->arg->relative = false;
        }
      } else {
        DebugPrintf(("It's local since it returned: %3d\n", pos));
        DebugPrintf(("Calculating sizes:\n position of stmt: %03d\n", (*j)->parent->offset));
        DebugPrintf((" size of statement and args: %03d\n", (*j)->parent->getSize()));
        DebugPrintf((" position of the arg relative to the stmt: %03d\n", (*j)->arg->relOffset));
        (*j)->arg->pointedPosition = pos - (*j)->arg->relOffset - (*j)->parent->offset;
        (*j)->arg->relative = true;
      }
    }
  }
}

void asm_program::assemble(const char * inputFile) {
  Bloat bytecode;
  bytecode.resize(tempOffset,0);
  Bloat::iterator pos = bytecode.begin();
  for (int i = 0; i < functions.size(); i++) {
    for(int j = 0; j < functions[i]->stmts.size(); j++) {
      functions[i]->stmts[j]->emitCode(pos);
    }
    for(int j = 0; j < functions[i]->locals.size(); j++) {
      functions[i]->locals[j]->emitCode(pos);
    }
  }
  for (int i = 0; i < globals.size(); i++) {
    globals[i]->emitCode(pos);
  }

  InfoPrintf(("Size of the generated file: %5d bytes in %4d cells\n",
              (int)bytecode.size()*4, (int)bytecode.size() ));
  for (int i = 0; i < bytecode.size(); i++) {
    DebugPrintf(("Mem %03d: %12d\n", i, bytecode[i]));
  }

  const int inputNameLen = strlen(inputFile);
  char outfile[inputNameLen + 3];
  strcpy(outfile, inputFile);
  outfile[inputNameLen-1] = 'b';
  outfile[inputNameLen] = 'i';
  outfile[inputNameLen+1] = 'n';
  outfile[inputNameLen+2] = '\0';

  BinWriter writer(outfile);
  writer.saveBinFileContent(bytecode);
}

