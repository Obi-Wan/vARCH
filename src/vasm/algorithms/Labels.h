/*
 * Labels.h
 *
 *  Created on: 27/lug/2011
 *      Author: ben
 */

#ifndef LABELS_H_
#define LABELS_H_

#include "../asm-classes.h"

///////////////////////
// Labels Management //
///////////////////////

typedef map<string, asm_label_statement *> LabelsMap;

class TableOfSymbols {
   LabelsMap defLabels;
public:
  void addLabel(asm_label_statement* lab);

  int getPositionOfLabel(const string & name) const
  {
    LabelsMap::const_iterator iter = defLabels.find(name);
    if (iter != defLabels.end()) {
      return iter->second->offset;
    } else return -1;
  }

  asm_label_statement * getStmt( const string & name) const
  {
    LabelsMap::const_iterator iter = defLabels.find(name);
    if (iter != defLabels.end()) {
      return iter->second;
    } else return NULL;
  }

  string emitDebugSymbols() const;
  string emitXMLDebugSymbols() const;
};

struct ArgLabelRecord {
  asm_statement * parent;
  asm_label_arg * arg;
};

#endif /* LABELS_H_ */
