%code top {
#include <stdio.h>
#include "asm-lexer.h"

void yyerror (YYLTYPE *locp, int *errorCode, char const *);
}

%code requires {
#include "../../include/parser_definitions.h"
#include "asm-parse-def.h"

int yyparse(int * errcode);
}

%locations
%define api.pure
%error-verbose
%parse-param {int *errorCode}

%union {
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
}

%token <integer> INTEGER
%token <integer> CONTENT_CONST
%token <real> REAL
%token <string> STRING
%token <instruction> INSTRUCTION
%token KEYWORD_INT KEYWORD_LONG KEYWORD_REAL KEYWORD_CHAR KEYWORD_STRING
%token KEYWORD_LOCAL KEYWORD_GLOBAL KEYWORD_FUNCTION KEYWORD_END
%token <label> DEF_LABEL
%token <label> POSITION_LABEL
%token <label> CONTENT_LABEL
%token <arg> REGISTER
%token <id> ID
%token COMA

%type <arg> arg
%type <stmt> stmt
%type <instr_stmt> instruction_stmt
%type <data_stmt> data_stmt
%type <keyw_stmt> data_keyword

%%

program
      : stmts
      | stmts functions
      | functions
      | stmts
        KEYWORD_GLOBAL { program.enterGlobalRegion() }
        data_stmts
      | stmts functions
        KEYWORD_GLOBAL { program.enterGlobalRegion() }
        data_stmts
      | functions
        KEYWORD_GLOBAL { program.enterGlobalRegion() }
        data_stmts
      ;

stmts
      : stmts stmt  { program.addStmt( $2 ) }
      | stmt        { program.addStmt( $1 ) }
      ;

functions
      : functions function
      | function
      ;

function
      : KEYWORD_FUNCTION ID { program.entryNewFunction( $2 ) }
        function_body
        KEYWORD_END         { program.closeCurrentFunction() }
      ;

function_body
      : stmts
        KEYWORD_LOCAL   { program.enterLocalRegionCurrentFunction() }
        data_stmts
      | stmts
      ;

arg
      : INTEGER         { $$ = new asm_immediate_arg( $1 , COST ) }
      | CONTENT_CONST   { $$ = new asm_immediate_arg( $1 , ADDR ) }
      | REAL            { $$ = new asm_immediate_arg( $1 ) }
      | POSITION_LABEL  { $$ = new asm_label_arg( $1 , COST ) }
      | CONTENT_LABEL   { $$ = new asm_label_arg( $1 , ADDR ) }
      | REGISTER        { $$ = $1 }
      ;

stmt
      : instruction_stmt  { $$ = $1 }
      | data_stmt         { $$ = $1 }
      ;

instruction_stmt
      : instruction_stmt arg  { $$ = $1->addArg( $2 ) }
      | INSTRUCTION           { $$ = new asm_instruction_statement( $1 ) }
      ;

data_stmts
      : data_stmts data_stmt  { program.addDataStmt( $2 ) }
      | data_stmt             { program.addDataStmt( $1 ) }
      ;

data_stmt
      : DEF_LABEL     { $$ = new asm_label_statement( $1 ) }
      | data_keyword  { $$ = $1 }
      ;

data_keyword
      : KEYWORD_INT INTEGER   { $$ = new asm_int_keyword_statement( $2 ) }
      | KEYWORD_LONG INTEGER  { $$ = new asm_long_keyword_statement( $2 ) }
      | KEYWORD_REAL REAL     { $$ = new asm_real_keyword_statement( $2 ) }
      | KEYWORD_CHAR ID       { $$ = new asm_char_keyword_statement( $2 ) }
      | KEYWORD_STRING STRING { $$ = new asm_string_keyword_statement( $2 ) }
      ;


%%

/*int
main (void) {
  int error = 0;
  int res = yyparse(&error);
  if (!res) {
    for(int i = 0; i < program.functions.size(); i++) {
      printf("Function: %s\n", program.functions[i]->name.c_str());
      for(int j = 0; j < program.functions[i]->stmts.size(); j++) {
        printf(" %s\n", program.functions[i]->stmts[j]->toString().c_str());
      }
      for(int j = 0; j < program.functions[i]->locals.size(); j++) {
        printf(" Local: %s\n", program.functions[i]->locals[j]->toString().c_str());
      }
    }
    for(int i = 0; i < program.globals.size(); i++) {
      printf("Global: %s\n", program.globals[i]->toString().c_str());
    }
  }
  program.assignValuesToLabels();
  program.assemble("prova.s");
  return res;
}*/

int
main(int argc, char** argv) {

  if (argc < 2) {
    printf("You didn't enter the ASM file to process\n");
    return (EXIT_FAILURE);
  }
  ++argv, --argc;
  yyin = fopen( argv[0], "r" );
  if (yyin != NULL) {
    try {
      int error = 0;
      int res = yyparse(&error);
      if (!res) {
        for(int i = 0; i < program.functions.size(); i++) {
          printf("Function: %s\n", program.functions[i]->name.c_str());
          for(int j = 0; j < program.functions[i]->stmts.size(); j++) {
            printf(" %s\n", program.functions[i]->stmts[j]->toString().c_str());
          }
          for(int j = 0; j < program.functions[i]->locals.size(); j++) {
            printf(" Local: %s\n", program.functions[i]->locals[j]->toString().c_str());
          }
        }
        for(int i = 0; i < program.globals.size(); i++) {
          printf("Global: %s\n", program.globals[i]->toString().c_str());
        }
      }
      program.assignValuesToLabels();
      program.assemble( argv[0] );
    } catch (BasicException e) {
      printf("Error: %s\n", e.what());
      return (EXIT_FAILURE);
    }
  } else {
    printf("I couldn't open the ASM file to process\n");
    return (EXIT_FAILURE);
  }
}


void
yyerror (YYLTYPE *locp, int *errorCode, char const *s) {
  fprintf(stderr, "ASSEMBLER ERROR: %s\n - at line: %4d col: %4d\n", s,
          locp->first_line, locp->first_column);
}


