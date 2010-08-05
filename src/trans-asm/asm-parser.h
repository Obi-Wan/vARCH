
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* "%code requires" blocks.  */

/* Line 1676 of yacc.c  */
#line 8 "asm-parser.y"

#include "parser_definitions.h"
#include "asm-parse-def.h"

int yyparse(int * errcode);



/* Line 1676 of yacc.c  */
#line 49 "asm-parser.h"

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INTEGER = 258,
     CONTENT_CONST = 259,
     REAL = 260,
     STRING = 261,
     INSTRUCTION = 262,
     KEYWORD_INT = 263,
     KEYWORD_LONG = 264,
     KEYWORD_REAL = 265,
     KEYWORD_CHAR = 266,
     KEYWORD_STRING = 267,
     KEYWORD_LOCAL = 268,
     KEYWORD_GLOBAL = 269,
     KEYWORD_FUNCTION = 270,
     KEYWORD_END = 271,
     DEF_LABEL = 272,
     POSITION_LABEL = 273,
     CONTENT_LABEL = 274,
     REGISTER = 275,
     ID = 276,
     COMA = 277
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 22 "asm-parser.y"

  char *string;
  int integer;
  float real;
  int instruction;
  char *id;
  char *label;

  struct asm_statement *stmt;
  struct asm_instruction_statement *instr_stmt;
  struct asm_data_statement *data_stmt;
  struct asm_data_keyword_statement *keyw_stmt;
  struct asm_arg *arg;



/* Line 1676 of yacc.c  */
#line 105 "asm-parser.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif



#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



