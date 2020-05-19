%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "linearflow.h"
	#include "linearflow.c"
	int yylex(void);
	extern FILE* yyin;
	

%}

%union{
	struct tnode *no;
	
}
%type <no> expr NUM program Slist Stmt InputStmt OutputStmt AsgStmt ID
%token NUM PLUS MINUS MUL DIV END ID READ WRITE ASSIGN_OP SEMICOLON START
%right ASSIGN_OP
%left PLUS MINUS
%left MUL DIV

%%

program : 	START Slist END SEMICOLON	{	$$ = $2;
											CodeGen($2,fout);
											evaluate($2);
											XsmExit();
											exit(1);
										}
			| START END SEMICOLON		{}
			;

expr 	: 	expr PLUS expr				{$$ = createTree(0,PLUS_OP,0,$1,$3);}
	 		| expr MINUS expr  			{$$ = createTree(0,MINUS_OP,0,$1,$3);}
	 		| expr MUL expr				{$$ = createTree(0,MUL_OP,0,$1,$3);}
	 		| expr DIV expr				{$$ = createTree(0,DIV_OP,0,$1,$3);}
	 		| '(' expr ')'				{$$ = $2;}
	 		| NUM						{$$ = $1;}
			| ID						{$$ = $1;}
	 		;

Slist 	: 	Slist Stmt 					{$$ = createTree(0,CONNECT_NODE,NULL,$1,$2);}
			| Stmt						{$$ = $1;}
			;

Stmt 	: 	InputStmt 					{$$ = $1;}
			| OutputStmt 				{$$ = $1;}
			| AsgStmt					{$$ = $1;}
			;

InputStmt : READ expr SEMICOLON			{$$ = createTree(0,READ_NODE,NULL,$2,NULL);}
			;

OutputStmt : WRITE expr SEMICOLON		{$$ = createTree(0,WRITE_NODE,NULL,$2,NULL);}
			;

AsgStmt : 	expr ASSIGN_OP expr SEMICOLON		{$$ = createTree(0,ASSIGN_NODE,NULL,$1,$3);}
			;
%%

yyerror(char const *s)
{
    printf("yyerror %s",s);
}


int main(void) {

	fout = fopen("linearflow.xsm","w");
	fin = fopen("linearflow.cod","r");
	yyin = fin;

	XsmHeader();

	yyparse();

	return 0;
}
