/*
 * IR_LowLevel_Defs.h
 *
 *  Created on: 29/lug/2011
 *      Author: ben
 */

#ifndef IR_LOWLEVEL_DEFS_H_
#define IR_LOWLEVEL_DEFS_H_

#include "macros.h"

class IncludesNode;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  IncludesNode * fileNode;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

# define YYLLOC_DEFAULT(Current, Rhs, N)\
    do\
      if (YYID (N))\
  {\
    (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;  \
    (Current).first_column = YYRHSLOC (Rhs, 1).first_column;\
    (Current).last_line    = YYRHSLOC (Rhs, N).last_line;   \
    (Current).last_column  = YYRHSLOC (Rhs, N).last_column; \
    (Current).fileNode     = YYRHSLOC (Rhs, N).fileNode;    \
  }\
      else\
  {\
    (Current).first_line   = (Current).last_line   =    \
      YYRHSLOC (Rhs, 0).last_line;        \
    (Current).first_column = (Current).last_column =    \
      YYRHSLOC (Rhs, 0).last_column;      \
    (Current).fileNode     = YYRHSLOC (Rhs, 0).fileNode;\
  }\
    while (YYID (0))

enum ObjType {
  ASM_ARG,
  ASM_FUNCTION_ARG,
  ASM_IMMEDIATE_ARG,
  ASM_LABEL_ARG,
  ASM_STATEMENT,
  ASM_INSTRUCTION_STATEMENT,
  ASM_FUNCTION_CALL,
  ASM_DATA_STATEMENT,
  ASM_LABEL_STATEMENT,
  ASM_KEYWORD_STATEMENT,
  ASM_INT_KEYWORD_STATEMENT,
  ASM_LONG_KEYWORD_STATEMENT,
  ASM_REAL_KEYWORD_STATEMENT,
  ASM_CHAR_KEYWORD_STATEMENT,
  ASM_STRING_KEYWORD_STATEMENT,
};

#endif /* IR_LOWLEVEL_DEFS_H_ */
