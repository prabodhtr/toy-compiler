%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "y.tab.h"
	#include "linearflow.h"
	int number;
	char* temp;
	int yyerror(char*);

%}

%%

[0-9]+	{number = atoi(yytext); yylval.no = createTree(number,0,NULL,NULL,NULL); return NUM;}
"read" 	{return READ;}
"write" {return WRITE;}
"begin" {return START;}
"end" 	{return END;}
[a-z]	{temp = strdup(yytext); yylval.no = createTree(0,1,temp,NULL,NULL); return ID;}
"+"		{return PLUS;}
"-"		{return MINUS;}
"*"		{return MUL;}
"/"		{return DIV;}
";" 	{return SEMICOLON;}
"=" 	{return ASSIGN_OP;}
[ \t\n]+ {}
[()]	{return *yytext;}	
.		{yyerror("unknown character\n");exit(1);}

%%

int yywrap(void) {
	if(feof(fin))
		return 1;
}
