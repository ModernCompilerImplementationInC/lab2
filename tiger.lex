%{
#include <string.h>
#include "util.h"
#include "tokens.h"
#include "errormsg.h"

int charPos=1;
char buf[100];
int size=0;
int yywrap(void)
{
 charPos=1;
 return 1;
}

void adjust(void)
{
 EM_tokPos=charPos;
 charPos+=yyleng;
}

void accum(char* c)
{

 buf[size]=*c;
  size++;
 
}
void cleanBuf()
{
 size=0;
}
string result()
{

if(size==0){
 string p=checked_malloc(7);
 memcpy(p,"(null)",6);
 return p;
}
for(int i=0;i<size-1;i++)
{
 if(buf[i]=='\\'&&buf[i+1]=='n'){
 for(int j=i+1;j<size-1;j++)
   buf[j]=buf[j+1];
 buf[i]='\n';
 size--;
 }
}
string p = checked_malloc(size+1);
 memcpy(p,buf,size);
 return p;

}
/*
* Please don't modify the lines above.
* You can add C declarations of your own below.
*/

%}
  /* You can add lex definitions here. */
letter [A-Za-z]
digit [0-9]
int {digit}+
id [A-Za-z][_A-Za-z0-9]*
delim [ \n\t]
whitespace {delim}+





%start COMMENT STR
%%
  /* 
  * Below are some examples, which you can wipe out
  * and write reguler expressions and actions of your own.
  */ 

<INITIAL>while {adjust();return WHILE;}
<INITIAL>for   {adjust();return FOR;}
<INITIAL>to {adjust();return TO;}
<INITIAL>break {adjust();return BREAK;}
<INITIAL>let {adjust();return LET;}
<INITIAL>in {adjust();return IN;}
<INITIAL>end {adjust();return END;}
<INITIAL>function {adjust();return FUNCTION;}
<INITIAL>var {adjust();return VAR;}
<INITIAL>type {adjust();return TYPE;}
<INITIAL>array {adjust();return ARRAY;}
<INITIAL>if {adjust();return IF;}
<INITIAL>then {adjust();return THEN;}
<INITIAL>else {adjust();return ELSE;}
<INITIAL>do {adjust();return DO;}
<INITIAL>of {adjust();return OF;}
<INITIAL>nil {adjust();return NIL;}
<INITIAL>{id}  {adjust();yylval.sval=String(yytext);return ID;}
<INITIAL>{int}  {adjust();yylval.ival=atoi(yytext);return INT;}
<INITIAL>":" {adjust();return COLON;}
<INITIAL>":=" {adjust();return ASSIGN;}
<INITIAL>\" {adjust();BEGIN STR;}
<INITIAL>{whitespace} {adjust();}
<INITIAL>"[" {adjust();return LBRACK;}
<INITIAL>"]" {adjust();return RBRACK;}
<INITIAL>"," {adjust();return COMMA;}
<INITIAL>";" {adjust();return SEMICOLON;}
<INITIAL>"(" {adjust();return LPAREN;}
<INITIAL>")" {adjust();return RPAREN;}
<INITIAL>"{" {adjust();return LBRACE;}
<INITIAL>"}" {adjust();return RBRACE;}
<INITIAL>"." {adjust();return DOT;}
<INITIAL>"+" {adjust();return PLUS;}
<INITIAL>"-" {adjust();return MINUS;}
<INITIAL>"*" {adjust();return TIMES;}
<INITIAL>"/" {adjust();return DIVIDE;}
<INITIAL>"<>" {adjust();return NEQ;}
<INITIAL>"<" {adjust();return LT;}
<INITIAL>"<=" {adjust();return LE;}
<INITIAL>">" {adjust();return GT;}
<INITIAL>">=" {adjust();return GE;}
<INITIAL>"&" {adjust();return AND;}
<INITIAL>"|" {adjust();return OR;}
<INITIAL>"="   {adjust();return EQ;}
<INITIAL>"/*" {adjust();BEGIN COMMENT;}
<COMMENT>\n    {adjust();}
<COMMENT>.    {adjust();}
<COMMENT>"*/" {adjust();BEGIN INITIAL;}
<STR>"\""  {charPos++;yylval.sval=result();;cleanBuf();BEGIN INITIAL; return STRING;}
<STR>.    {charPos++;accum(yytext);}








