<?
$domDocument = new DOMDocument();
$domDocument->load("instructions.xml");

print <<<EOT
/**/

%option noyywrap

%{
#include <stdio.h>

#define MAX_STR_CONST 256
%}

%s STRING
%s COMMENT

DIGIT    [0-9]
LETTER   [a-zA-Z]
PUNCT    [-!"#$%&'()*+,./:;<=>?@[\\\\\\]^_`{|}~]
LETT_DIG ({LETTER}|{DIGIT})
ID       {LETTER}{LETT_DIG}*
KEYBKEYS ([ \\t]|{LETT_DIG}|[-!#$%&'()*+,./:;<=>?@[\\\\\\]^_`{|}~])

LABEL    \\.{ID}\\:
AT_LABEL \\@{ID}
TO_LABEL \\.{ID}

REGISTER \\%([RA][1-8]|[U]?SP)
ADDR_REG \\(([RA][1-8]|[U]?SP)\\)

INTEGER  \\\${DIGIT}+
FLOAT    \\\$({DIGIT}+\\.{DIGIT}*|.{DIGIT}+)
STRING   \\"{KEYBKEYS}+\\"

%%
             char string_buf[MAX_STR_CONST];
             char *string_buf_ptr;

<INITIAL>{
 {INTEGER}   fprintf( yyout, "INTEGER(%d) ", atoi(yytext+1) );
 {FLOAT}     fprintf( yyout, "FLOAT(%g) ", atof(yytext+1) );

EOT;
$output = "\n";
$cpuInstrs = $domDocument->getElementsByTagName("cpu")->item(0);
foreach ($cpuInstrs->childNodes as $instr) {
  if ($instr->hasChildNodes()) {
    $instrName;
    $instrVal;
    $instrCode;
    foreach ($instr->childNodes as $node) {
      switch ($node->nodeName) {
        case "name":
          $instrName = $node->textContent;
          break;
        case "value":
          $instrVal = $node->textContent;
          break;
        case "code":
          $instrCode = $node->textContent;
          break;
      }
    }
    $output .= "{$instrName}|";
    unset ($instrName);
    unset ($instrVal);
    unset ($instrCode);
  }
}
$output = substr($output,0, strlen($output)-1 ) . " {\n";
print $output;

print <<<EOT
             fprintf( yyout, "INSTRUCTION(%s) ", yytext );
           }
     
 \.int|\.string|\.local|\.global {
             fprintf( yyout, "KEYWORD(%s) ", yytext );
           }
     
 {LABEL}     fprintf( yyout, "DEF_LABEL(%s) ", yytext );
 {TO_LABEL}  fprintf( yyout, "REF_LABEL(%s) ", yytext );
 {AT_LABEL}  fprintf( yyout, "ADDR_LABEL(%s) ", yytext );

 {REGISTER}  fprintf( yyout, "REG(%s) ", yytext );
 {ADDR_REG}  fprintf( yyout, "ADDR_REG(%s) ", yytext );

 {ID}        fprintf( yyout, "ID(%s) ", yytext );
 
 "+"|"-"|"*"|"/" {
             fprintf( yyout, "OP(%s) ", yytext );
           }

 "/*"        BEGIN(COMMENT);

 ";".*\\n     fprintf( yyout, "\\n" );

 \" {
             string_buf_ptr = string_buf;
             BEGIN(STRING);
	   }

 \\n          fprintf( yyout, "\\n" );

 {DIGIT}+{ID} {
	     fprintf( yyout, "Error, unknown symbol: %s\\n", yytext );
	     exit(1);
	   }
 [ \\t]+          /* eat up whitespace */

}

<COMMENT>{
 \\n          fprintf( yyout, "\\n" );
 "*/"        BEGIN(INITIAL);
 .           /* eat up */
}

<STRING>{
 \\\\(.|\\n) {
             *string_buf_ptr++ = yytext[0];
             *string_buf_ptr++ = yytext[1];
	   }
 
 [^\\\\\\n\\"]+ {
             char *yptr = yytext;
             while ( *yptr )
               *string_buf_ptr++ = *yptr++;
           }

 \\" {
             BEGIN(INITIAL);
             *string_buf_ptr = '\\0';
	     fprintf( yyout, "STRING(%s) ", string_buf );
	   }
}

%%

int
main( int argc, char **argv ) {
  ++argv, --argc;  /* skip over program name */
  if ( argc > 0 ) {
    yyin = fopen( argv[0], "r" );
  } else {
    yyin = stdin;
  }
  if ( argc > 1 ) {
    yyout = fopen( argv[1], "w" );
  } else {
    yyout = stdout;
  }
  yylex();
  fprintf( yyout, "\\n" );
  return 0;
}

EOT;
print "\n";

?>
