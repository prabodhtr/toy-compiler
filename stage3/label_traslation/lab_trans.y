%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "lab_trans.h"
	#include "lab_trans.c"
	int yylex(void);
	extern FILE* yyin;
	extern char* yytext;

%}

%union{
	struct tnode *no;
	
}
%type <no> expr NUM program Slist Stmt InputStmt OutputStmt AsgStmt Ifstmt Whilestmt Dowhilestmt ID 
%token NUM PLUS MINUS MUL DIV END ID READ WRITE ASSIGN_OP SEMICOLON START
%token IF THEN ELSE ENDIF DO WHILE ENDWHILE LT GE GT LE EQ NEQ BREAK CONTINUE
%right ASSIGN_OP
%left LT GT LE GE EQ NE
%left PLUS MINUS
%left MUL DIV

%%

program : 	START Slist END SEMICOLON	{	$$ = $2;
											CodeGen($2,fout);
											XsmExit();
											evaluate($2);
											exit(1);
										}
			| START END SEMICOLON		{}
			;


			;

expr 	: 	expr PLUS expr				{$$ = createTree(0,PLUS_OP,0,INTTYPE,$1,NULL,$3);}
	 		| expr MINUS expr  			{$$ = createTree(0,MINUS_OP,0,INTTYPE,$1,NULL,$3);}
	 		| expr MUL expr				{$$ = createTree(0,MUL_OP,0,INTTYPE,$1,NULL,$3);}
	 		| expr DIV expr				{$$ = createTree(0,DIV_OP,0,INTTYPE,$1,NULL,$3);}
	 		| '(' expr ')'				{$$ = $2;}
	 		| expr LT expr				{$$ = createTree(0,RELOP_LT,0,BOOLTYPE,$1,NULL,$3);}
			| expr GT expr				{$$ = createTree(0,RELOP_GT,0,BOOLTYPE,$1,NULL,$3);}
			| expr LE expr				{$$ = createTree(0,RELOP_LE,0,BOOLTYPE,$1,NULL,$3);}
			| expr GE expr				{$$ = createTree(0,RELOP_GE,0,BOOLTYPE,$1,NULL,$3);}	
			| expr NE expr				{$$ = createTree(0,RELOP_NE,0,BOOLTYPE,$1,NULL,$3);}
			| expr EQ expr				{$$ = createTree(0,RELOP_EQ,0,BOOLTYPE,$1,NULL,$3);}
			| ID						{$$ = $1;}
	 		| NUM						{$$ = $1;}
			;

Slist 	: 	Slist Stmt 					{$$ = createTree(0,CONNECT_NODE,NULL,-1,$1,NULL,$2);}
			| Stmt						{$$ = $1;}			
			;

Stmt 	: 	InputStmt 					{$$ = $1;}
			| OutputStmt 				{$$ = $1;}
			| AsgStmt					{$$ = $1;}
			| BREAK SEMICOLON			{$$ = createTree(0,BREAK_NODE,NULL,-1,NULL,NULL,NULL);}
			| CONTINUE SEMICOLON		{$$ = createTree(0,CONTINUE_NODE,NULL,-1,NULL,NULL,NULL);}
			| Ifstmt					{$$ = $1;}
			| Whilestmt					{$$ = $1;}
			| Dowhilestmt				{$$ = $1;}
			;

Ifstmt 	: 	IF expr THEN Slist ELSE Slist ENDIF		{$$ = createTree(0,IF_COND,NULL,BOOLTYPE,$2,$4,$6);}
			| IF expr THEN Slist ENDIF				{$$ = createTree(0,IF_COND,NULL,BOOLTYPE,$2,$4,NULL);}
			;

Whilestmt 	: WHILE expr DO Slist ENDWHILE		{$$ = createTree(0,WHILE_LOOP,NULL,BOOLTYPE,$2,NULL,$4);}

Dowhilestmt	: DO Slist WHILE expr				{$$ = createTree(0,DO_WHILE,NULL,BOOLTYPE,$4,NULL,$2);}

InputStmt 	: READ expr SEMICOLON			{$$ = createTree(0,READ_NODE,NULL,-1,$2,NULL,NULL);}
			;

OutputStmt : WRITE expr SEMICOLON		{$$ = createTree(0,WRITE_NODE,NULL,-1,$2,NULL,NULL);}
			;

AsgStmt : 	expr ASSIGN_OP expr SEMICOLON		{$$ = createTree(0,ASSIGN_NODE,NULL,-1,$1,NULL,$3);}
			;
%%

yyerror(char const *s)
{
    printf("yyerror %s",yytext);
}


int main(void) {

	fout = fopen("lab_trans.xsm","w");
	fin = fopen("lab_trans.cod","r");
	yyin = fin;

	XsmHeader();

	yyparse();

	return 0;
}
