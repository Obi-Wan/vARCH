/*
 * Labels.h
 *
 *  Created on: 27/lug/2011
 *      Author: ben
 */

#ifndef LABELS_H_
#define LABELS_H_

#include "../IR/IR_LowLevel_Statements.h"

#include <map>

using namespace std;

///////////////////////
// Labels Management //
///////////////////////

typedef map<string, asm_label_statement *> LabelsMap;

class TableOfSymbols {
   LabelsMap defLabels;
public:
  void addLabel(asm_label_statement* lab);

  void importLabels(const TableOfSymbols & o);

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

  asm_label_statement * getStmt( const string & name, const string & prefix) const
  {
    return this->getStmt(prefix + "::" + name);
  }

  string emitDebugSymbols() const;
  string emitXMLDebugSymbols() const;

  const LabelsMap & getLabels() const throw() { return defLabels; }
};

struct ArgLabelRecord {
  asm_statement * parent;
  asm_label_arg * arg;
};

#endif /* LABELS_H_ */
