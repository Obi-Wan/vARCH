<?
$domDocument = new DOMDocument();
$domDocument->load("instructions.xml");

print <<<EOT
/**/

%option noyywrap

%{
#include <stdio.h>
%}

DIGIT    [0-9]
LETTER   [a-zA-Z]
PUNCT    [-!"#$%&'()*+,./:;<=>?@[\\\\\\]^_`{|}~]
LETT_DIG ({LETTER}|{DIGIT})
ID       {LETTER}{LETT_DIG}*
KEYBKEYS ([ \\t]|{LETT_DIG}|[-!#$%&'()*+,./:;<=>?@[\\\\\\]^_`{|}~])

LABEL    \\.{ID}+\\:
AT_LABEL \\@{ID}+
TO_LABEL \\.{ID}+

REGISTER \\%([RA][1-8]|[U]?SP)
ADDR_REG \\(([RA][1-8]|[U]?SP)\\)

INTEGER  \\\${DIGIT}+
FLOAT    \\\$({DIGIT}+\\.{DIGIT}*|.{DIGIT}+)
STRING   \\"{KEYBKEYS}+\\"

%%

{INTEGER}   fprintf( yyout, "INTEGER:     %s (%d)\\n", yytext, atoi(yytext+1) );
{FLOAT}     fprintf( yyout, "FLOAT:       %s (%g)\\n", yytext, atof(yytext+1) );
{STRING}    fprintf( yyout, "STRING:      %s\\n", yytext );

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
$output = substr($output,0, strlen($output)-1 ) . " {";
print $output;

print <<<EOT
            fprintf( yyout, "INSTRUCTION: %s\\n", yytext );
          }
     
\.int|\.long|\.char|\.string|\.local|\.global|\.function|\.end {
            fprintf( yyout, "KEYWORD:     %s\\n", yytext );
          }
     
{LABEL}     fprintf( yyout, "DEF_LABEL:   %s\\n", yytext );
{TO_LABEL}  fprintf( yyout, "REF_LABEL:   %s\\n", yytext );
{AT_LABEL}  fprintf( yyout, "ADDR_LABEL:  %s\\n", yytext );
     
{REGISTER}  fprintf( yyout, "REG:         %s\\n", yytext );
{ADDR_REG}  fprintf( yyout, "ADDR_REG:    %s\\n", yytext );
     
{ID}        fprintf( yyout, "ID:          %s\\n", yytext );
 
"+"|"-"|"*"|"/" {
            fprintf( yyout, "OP:          %s\\n", yytext );
	  }
     
"{"[\^{}}\\n]*"}"  /* eat up one-line comments */

^[ \\t]*";".*\\n	  /* one line comments*/
     
[ \\t\\n]+          /* eat up whitespace */


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
  return 0;
}

EOT;
print "\n";

?>
