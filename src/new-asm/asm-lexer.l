/**/
%top{
#include "asm-parser.h"
#include <cstdlib>

using namespace std;
}

%s STRING_CONTEXT
%s COMMENT

DIGIT     [0-9]
LETTER    [a-zA-Z]
PUNCT     [-!"#$%&'()*+,./:;<=>?@[\\\]^_`{|}~]
LETT_DIG  ({LETTER}|{DIGIT})
ID        {LETTER}{LETT_DIG}*
KEYBKEYS  ([ \t]|{LETT_DIG}|[-!#$%&'()*+,./:;<=>?@[\\\]^_`{|}~])

LABEL     \.{ID}\:
AT_LABEL  \@{ID}
TO_LABEL  \.{ID}

STK_PTR   SP
USTK_PTR  USP
DREG      R[1-8]
AREG      A[1-8]
SREG      SR

REGISTER  \%([RA][1-8]|[U]?SP)
ADDR_REG  r?\(([RA][1-8]|[U]?SP)\)

%option bison-bridge bison-locations noyywrap
%option outfile="asm-lexer.cpp" header-file="asm-lexer.h"

%{

#define MAX_STR_CONST 256

char string_buf[MAX_STR_CONST];
char *string_buf_ptr;

int getRegNum(const char * str);
void createRegArg(asm_arg *& arg, const char * str, const TypeOfArgument& type,
                  const int& offset, const bool rel);
void createConstIntArg(asm_arg *& arg, const int _val);
void createConstRealArg(asm_arg *& arg, const float _val);
void createSpecialRegArg(asm_arg *& arg, const enum Registers& reg);
void createSpecialAddrRegArg(asm_arg *& arg, const enum Registers& reg,
                              const bool rel);


#define YY_USER_ACTION							\
   do {									\
      yylloc->first_column = yylloc->last_column;			\
      yylloc->last_column += yyleng;					\
   } while(0);

%}

%%

<INITIAL>{
\${DIGIT}+ {
            yylval->integer = atoi(yytext+1);
            return INTEGER;
          }
\$({DIGIT}+\.{DIGIT}*|.{DIGIT}+) {
            yylval->real = atof(yytext+1);
            return REAL;
          }

SLEEP|PUSHA|POPA|RET|RETEX|REBOOT|HALT|NOT|INCR|DECR|COMP2|LSH|RSH|STACK|PUSH|POP|JSR|JMP|IFJ|IFNJ|TCJ|TZJ|TOJ|TNJ|TSJ|MOV|ADD|MULT|SUB|DIV|QUOT|AND|OR|XOR|MMU|PUT|GET|EQ|LO|MO|LE|ME|NEQ|BPUT|BGET|IFEQJ|IFNEQJ|IFLOJ|IFMOJ|IFLEJ|IFMEJ {
            yylval->instruction = ISet.getIstr(yytext);
            return INSTRUCTION;
          }

\.int       return KEYWORD_INT;
\.long      return KEYWORD_LONG;
\.real      return KEYWORD_REAL;
\.char      return KEYWORD_CHAR;
\.string    return KEYWORD_STRING;
\.local     return KEYWORD_LOCAL;
\.global    return KEYWORD_GLOBAL;
\.function  return KEYWORD_FUNCTION;
\.end       return KEYWORD_END;

\>{DIGIT}   yylval->integer = atoi(yytext+1); return CONTENT_CONST;
{LABEL}     yytext[yyleng-1] = '\0'; yylval->label = yytext+1; return DEF_LABEL;
{TO_LABEL}  yylval->label = yytext+1; return CONTENT_LABEL;
{AT_LABEL}  yylval->label = yytext+1; return POSITION_LABEL;

\%{SREG} {
            createSpecialRegArg(yylval->arg, STATE_REGISTER);
            return REGISTER;
          }
\%{STK_PTR} {
            createSpecialRegArg(yylval->arg, STACK_POINTER);
            return REGISTER;
          }
\%{USTK_PTR} {
            createSpecialRegArg(yylval->arg, USER_STACK_POINTER);
            return REGISTER;
          }
\%{DREG} {
            createRegArg(yylval->arg, yytext+2, REG, 0, false);
            return REGISTER;
          }
\%{AREG} {
            createRegArg(yylval->arg, yytext+2, REG, OFFSET_REGS, false);
            return REGISTER;
          }
\+\%{DREG} {
            createRegArg(yylval->arg, yytext+3, REG_PRE_INCR, 0, false);
            return REGISTER;
          }
\+\%{AREG} {
            createRegArg(yylval->arg, yytext+3, REG_PRE_INCR, OFFSET_REGS, false);
            return REGISTER;
          }
\-\%{DREG} {
            createRegArg(yylval->arg, yytext+3, REG_PRE_DECR, 0, false);
            return REGISTER;
          }
\-\%{AREG} {
            createRegArg(yylval->arg, yytext+3, REG_PRE_DECR, OFFSET_REGS, false);
            return REGISTER;
          }
\%{DREG}\+ {
            createRegArg(yylval->arg, yytext+2, REG_POST_INCR, 0, false);
            return REGISTER;
          }
\%{AREG}\+ {
            createRegArg(yylval->arg, yytext+2, REG_POST_INCR, OFFSET_REGS, false);
            return REGISTER;
          }
\%{DREG}\- {
            createRegArg(yylval->arg, yytext+2, REG_POST_DECR, 0, false);
            return REGISTER;
          }
\%{AREG}\- {
            createRegArg(yylval->arg, yytext+2, REG_POST_DECR, OFFSET_REGS, false);
            return REGISTER;
          }

r?\({SREG}\) {
            createSpecialAddrRegArg(yylval->arg, STATE_REGISTER,
                                    (*yytext) == 'r');
            return REGISTER;
          }
r?\({STK_PTR}\) {
            createSpecialAddrRegArg(yylval->arg, STACK_POINTER,
                                    (*yytext) == 'r');
            return REGISTER;
          }
r?\({USTK_PTR}\) {
            createSpecialAddrRegArg(yylval->arg, USER_STACK_POINTER,
                                    (*yytext) == 'r');
            return REGISTER;
          }
r?\({DREG}\) {
            createRegArg(yylval->arg, yytext+((*yytext) == 'r' ? 3 : 2),
                          ADDR_IN_REG, 0, (*yytext) == 'r');
            return REGISTER;
          }
r?\({AREG}\) {
            createRegArg(yylval->arg, yytext+((*yytext) == 'r' ? 3 : 2),
                          ADDR_IN_REG, OFFSET_REGS, (*yytext) == 'r');
            return REGISTER;
          }
\+r?\({DREG}\) {
            createRegArg(yylval->arg, yytext+((*yytext) == 'r' ? 4 : 3),
                          ADDR_IN_REG_PRE_INCR, 0, (*yytext) == 'r');
            return REGISTER;
          }
\+r?\({AREG}\) {
            createRegArg(yylval->arg, yytext+((*yytext) == 'r' ? 4 : 3),
                          ADDR_IN_REG_PRE_INCR, OFFSET_REGS, (*yytext) == 'r');
            return REGISTER;
          }
\-r?\({DREG}\) {
            createRegArg(yylval->arg, yytext+((*yytext) == 'r' ? 4 : 3),
                          ADDR_IN_REG_PRE_DECR, 0, (*yytext) == 'r');
            return REGISTER;
          }
\-r?\({AREG}\) {
            createRegArg(yylval->arg, yytext+((*yytext) == 'r' ? 4 : 3),
                          ADDR_IN_REG_PRE_DECR, OFFSET_REGS, (*yytext) == 'r');
            return REGISTER;
          }
r?\({DREG}\)\+ {
            createRegArg(yylval->arg, yytext+((*yytext) == 'r' ? 3 : 2),
                          ADDR_IN_REG_POST_INCR, 0, (*yytext) == 'r');
            return REGISTER;
          }
r?\({AREG}\)\+ {
            createRegArg(yylval->arg, yytext+((*yytext) == 'r' ? 3 : 2),
                          ADDR_IN_REG_POST_INCR, OFFSET_REGS, (*yytext) == 'r');
            return REGISTER;
          }
r?\({DREG}\)\- {
            createRegArg(yylval->arg, yytext+((*yytext) == 'r' ? 3 : 2),
                          ADDR_IN_REG_POST_DECR, 0, (*yytext) == 'r');
            return REGISTER;
          }
r?\({AREG}\)\- {
            createRegArg(yylval->arg, yytext+((*yytext) == 'r' ? 3 : 2),
                          ADDR_IN_REG_POST_DECR, OFFSET_REGS, (*yytext) == 'r');
            return REGISTER;
          }

{ID}        yylval->id = yytext; return ID;

"/*"        BEGIN(COMMENT);

\" {
            string_buf_ptr = string_buf;
            BEGIN(STRING_CONTEXT);
          }

";".*\n|\n {
            yylloc->first_line++;
            yylloc->first_column = 1;
            yylloc->last_line++;
            yylloc->last_column = 1;
            return END_LINE;
          }

{DIGIT}+{ID} {
            fprintf( stderr, "Lexing Error, unknown symbol: %s\n", yytext );
            exit(1);
          }
[ \t]+      /* eat up whitespace */
,           return COMA;

. {
            fprintf( stderr, "Lexing Error, unknown symbol: %s\n", yytext );
            exit(1);
          }
}

<COMMENT>{
\n  {
            yylloc->first_line++;
            yylloc->first_column = 1;
            yylloc->last_line++;
            yylloc->last_column = 1;
            return END_LINE;
          }
"*/"        BEGIN(INITIAL);
.           /* eat up */
}

<STRING_CONTEXT>{
\\n         *string_buf_ptr++ = '\n';
\\t         *string_buf_ptr++ = '\t';
\\r         *string_buf_ptr++ = '\r';
\\b         *string_buf_ptr++ = '\b';
\\f         *string_buf_ptr++ = '\f';

\\(.|\n) {
            *string_buf_ptr++ = yytext[0];
            *string_buf_ptr++ = yytext[1];
          }

[^\\\n\"]+ {
            char *yptr = yytext;
            while ( *yptr )
              *string_buf_ptr++ = *yptr++;
          }

\" {
            BEGIN(INITIAL);
            *string_buf_ptr = '\0';
            yylval->string = string_buf;
            return STRING;
          }
}

%%

int getRegNum(const char * str) {
  char * tempStr = strndup(str,1);
  int num = atoi(tempStr) -1;
  free(tempStr);
  return num;
}

void createRegArg(asm_arg *& arg, const char * str, const TypeOfArgument& type,
                  const int& offset, const bool rel)
{
  asm_immediate_arg * tempArg = new asm_immediate_arg();
  tempArg->relative = rel;
  tempArg->type = type;
  tempArg->content.val = getRegNum(str) + offset;
  arg = tempArg;
}
void createSpecialRegArg(asm_arg *& arg, const enum Registers& reg) {
  asm_immediate_arg * tempArg = new asm_immediate_arg();
  tempArg->type = REG;
  tempArg->content.regNum = reg;
  tempArg->relative = false;
  arg = tempArg;
}
void createSpecialAddrRegArg(asm_arg *& arg, const enum Registers& reg,
                              const bool rel) {
  asm_immediate_arg * tempArg = new asm_immediate_arg();
  tempArg->type = ADDR_IN_REG;
  tempArg->content.regNum = reg;
  tempArg->relative = rel;
  arg = tempArg;
}
void createConstIntArg(asm_arg *& arg, const int _val) {
  asm_immediate_arg * tempArg = new asm_immediate_arg();
  tempArg->type = COST;
  tempArg->content.val = _val;
  tempArg->relative = false;
  arg = tempArg;
}
void createConstRealArg(asm_arg *& arg, const float _val) {
  asm_immediate_arg * tempArg = new asm_immediate_arg();
  tempArg->type = COST;
  tempArg->content.fval = _val;
  tempArg->relative = false;
  arg = tempArg;
}
