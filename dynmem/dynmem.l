%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "y.tab.h"
	#include "dynmem.h"
	int number;
	char* temp;
	int yyerror(char const*);

%}

%%

[0-9]+			{number = atoi(yytext); yylval.no = createTree(number,CONSTANT,NULL,INTTYPE,NULL,NULL,NULL,NULL); return NUM;}
\"[A-Za-z0-9" "]+\" 		{temp = strdup(yytext); yylval.no = createTree(0,CONSTANT,temp,STRTYPE,NULL,NULL,NULL,NULL); return STRING;}
"read" 			{return READ;}
"write" 		{return WRITE;}
"main"			{return MAIN;}	
"begin" 		{return START;}
"end" 			{return END;}
"if"			{return IF;}
"then"			{return THEN;}
"else"			{return ELSE;}
"do"			{return DO;}
"while"			{return WHILE;}
"endif"			{return ENDIF;}
"endwhile" 		{return ENDWHILE;}
"break"			{return BREAK;}
"continue"		{return CONTINUE;}
"return"		{return RETURN;}
"decl"			{return DECL;}
"enddecl"		{return ENDDECL;}
"type"			{return TYPE;}
"endtype"		{return ENDTYPE;}
"Heapset"		{return HEAPSET;}
"Alloc"			{return ALLOC;}
"Free"			{return FREE;}
"breakpoint"	{return BREAKPOINT;}
"int"			{yylval.no = createTree(INTTYPE,TYPE_NODE,"INT",-1,NULL,NULL,NULL,NULL);return INT;}
"char"			{yylval.no = createTree(STRTYPE,TYPE_NODE,"STR",-1,NULL,NULL,NULL,NULL);return STR;}
"null"			{yylval.no = createTree(NULLTYPE,TYPE_NODE,"NULL",-1,NULL,NULL,NULL,NULL);return NUL;}
"bool"			{yylval.no = createTree(BOOLTYPE,TYPE_NODE,"BOOL",-1,NULL,NULL,NULL,NULL);return BOOL;}
"void"			{yylval.no = createTree(VOIDTYPE,TYPE_NODE,"VOID",-1,NULL,NULL,NULL,NULL);return VOID;}
[a-zA-Z]+[a-zA-Z0-9]*	{temp = strdup(yytext); yylval.no = createTree(0,VAR,temp,-1,NULL,NULL,NULL,NULL); return ID;} //optype of VAR is assigned only during codeGen()
"&"				{return ADDR;}
"+"				{return PLUS;}
"-"				{return MINUS;}
"*"				{return MUL;}
"/"				{return DIV;}
"%"				{return MOD;}
";" 			{return SEMICOLON;}
"=" 			{return ASSIGN_OP;}
"<"				{return LT;}
"<="			{return LE;}
">"				{return GT;}
">="			{return GE;}
"=="			{return EQ;}
"!="			{return NE;}
","				{return *yytext;}
[ \t\n]+ 		{}
[(){}.]			{return *yytext;}
[\[\]]			{return *yytext;}
.				{yyerror("unknown character\n");exit(1);}

%%

int yywrap(void) {
	if(feof(fin))
		return 1;
}
