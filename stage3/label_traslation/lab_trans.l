%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "y.tab.h"
	#include "lab_trans.h"
	int number;
	char* temp;
	int yyerror(char*);

%}

%%

[0-9]+	{number = atoi(yytext); yylval.no = createTree(number,CONSTANT,NULL,INTTYPE,NULL,NULL,NULL); return NUM;}
"read" 	{return READ;}
"write" {return WRITE;}
"begin" {return START;}
"end" 	{return END;}
"if"	{return IF;}
"then"	{return THEN;}
"else"	{return ELSE;}
"do"	{return DO;}
"while"	{return WHILE;}
"endif"	{return ENDIF;}
"endwhile" {return ENDWHILE;}
"break"	{return BREAK;}
"continue"	{return CONTINUE;}
[a-z]	{temp = strdup(yytext); yylval.no = createTree(0,VAR,temp,INTTYPE,NULL,NULL,NULL); return ID;}
"+"		{return PLUS;}
"-"		{return MINUS;}
"*"		{return MUL;}
"/"		{return DIV;}
";" 	{return SEMICOLON;}
"=" 	{return ASSIGN_OP;}
"<"		{return LT;}
"<="	{return LE;}
">"		{return GT;}
">="	{return GE;}
"=="	{return EQ;}
"!="	{return NE;}
[ \t\n]+ {}
[()]	{return *yytext;}	
.		{yyerror("unknown character\n");exit(1);}

%%

int yywrap(void) {
	if(feof(fin))
		return 1;
}
