/* 
 * File:   parser_definitions.h
 * Author: ben
 *
 * Created on 11 dicembre 2009, 0.03
 */

#ifndef _PARSER_DEFINITIONS_H
#define	_PARSER_DEFINITIONS_H

#include "asm_helpers.h"

#include <string>
#include <vector>
#include <map>
#include <sstream>

using namespace std;

typedef map<string, unsigned int> Labels;
typedef map<string, int> Istructions;
typedef vector<int> Constants;
typedef vector<pair< int, vector<string> > > CodeLines;


#endif	/* _PARSER_DEFINITIONS_H */

