%{
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	int yyerror(char const*);
	#include "sym_table.c"
	int yylex(void);
	extern FILE* yyin;
	extern char* yytext;
	int type_enc;

%}

%union{
	struct tnode *no;
	
}
%type <no> expr NUM program Slist Stmt InputStmt OutputStmt AsgStmt Ifstmt Whilestmt Dowhilestmt ID STRING POINTER ADDR
%token NUM PLUS MINUS MUL DIV MOD END ID READ WRITE ASSIGN_OP SEMICOLON START INT STR DECL ENDDECL STRING POINTER ADDR
%token IF THEN ELSE ENDIF DO WHILE ENDWHILE LT GE GT LE EQ NEQ BREAK CONTINUE ADR
%right ASSIGN_OP
%left LT GT LE GE EQ NE
%left PLUS MINUS
%left MUL DIV MOD

%%

program 		: Declarations START Slist END SEMICOLON	{$$ = $3;
															XsmHeader();
															CodeGen($3,fout);
															XsmExit();
															//evaluate($3);
															exit(1);
														}
				| Declarations START END SEMICOLON					{}
				;

Declarations	: DECL DeclList ENDDECL 				{}
				| DECL ENDDECL							{}
				;

DeclList 		: DeclList Decl 						{}
				| Decl									{}
				;

Decl 			: Type VarList SEMICOLON				{}
				;

Type 			: INT 									{type_enc = INTTYPE;}
				| STR									{type_enc = STRTYPE;}
				;

VarList 		: VarList ',' ID '[' NUM ']' '[' NUM ']'{Install(($3)->varname,type_enc,($5)->val,$8->val);}
				| VarList ',' ID '[' NUM ']'			{Install(($3)->varname,type_enc,($5)->val,1);}
				| VarList ',' ID 						{Install(($3)->varname,type_enc,1,1);}
				| VarList ',' MUL ID					{if(type_enc == INTTYPE)
															Install(($4)->varname,INTPTR,1,1);
														else
															Install(($4)->varname,STRPTR,1,1);}
				| ID '[' NUM ']' '[' NUM ']'			{Install(($1)->varname,type_enc,($3)->val,($6)->val);}
				| ID '[' NUM ']'						{Install(($1)->varname,type_enc,($3)->val,1);}
				| ID									{Install(($1)->varname,type_enc,1,1);}
				| MUL ID								{if(type_enc == INTTYPE)
															Install(($2)->varname,INTPTR,1,1);
														else
															Install(($2)->varname,STRPTR,1,1);}
				;

expr 			: expr PLUS expr						{$$ = createTree(0,PLUS_OP,0,INTTYPE,$1,NULL,$3);}
	 			| expr MINUS expr  						{$$ = createTree(0,MINUS_OP,0,INTTYPE,$1,NULL,$3);}
	 			| expr MUL expr							{$$ = createTree(0,MUL_OP,0,INTTYPE,$1,NULL,$3);}
	 			| expr DIV expr							{$$ = createTree(0,DIV_OP,0,INTTYPE,$1,NULL,$3);}
				| expr MOD expr							{$$ = createTree(0,MODULO_OP,0,INTTYPE,$1,NULL,$3);}	
	 			| '(' expr ')'							{$$ = $2;}
	 			| expr LT expr							{$$ = createTree(0,RELOP_LT,0,BOOLTYPE,$1,NULL,$3);}
				| expr GT expr							{$$ = createTree(0,RELOP_GT,0,BOOLTYPE,$1,NULL,$3);}
				| expr LE expr							{$$ = createTree(0,RELOP_LE,0,BOOLTYPE,$1,NULL,$3);}
				| expr GE expr							{$$ = createTree(0,RELOP_GE,0,BOOLTYPE,$1,NULL,$3);}	
				| expr NE expr							{$$ = createTree(0,RELOP_NE,0,BOOLTYPE,$1,NULL,$3);}
				| expr EQ expr							{$$ = createTree(0,RELOP_EQ,0,BOOLTYPE,$1,NULL,$3);}
				| ID '[' expr ']' '[' expr ']'			{$1->left = $3;$1->right = $6;check($1);$$ = $1;}
				| ID '[' expr ']'						{$1->left = $3;check($1);$$ = $1;}
				| ID									{check($1);$$ = $1;}
	 			| NUM									{$$ = $1;}
				| STRING								{$$ = $1;}	
				| ADR ID								{$2->nodetype = ADDRESS_NODE; $$ = $2;}
				| MUL ID								{$2->nodetype = POINTER_NODE;$$ = $2;}
				;

Slist 			: Slist Stmt 							{$$ = createTree(0,CONNECT_NODE,NULL,-1,$1,NULL,$2);}
				| Stmt									{$$ = $1;}
				;

Stmt 			: InputStmt 							{$$ = $1;}
				| OutputStmt 							{$$ = $1;}
				| AsgStmt								{$$ = $1;}
				| BREAK SEMICOLON						{$$ = createTree(0,BREAK_NODE,NULL,-1,NULL,NULL,NULL);}
				| CONTINUE SEMICOLON					{$$ = createTree(0,CONTINUE_NODE,NULL,-1,NULL,NULL,NULL);}
				| Ifstmt								{$$ = $1;}
				| Whilestmt								{$$ = $1;}
				| Dowhilestmt							{$$ = $1;}
				;


Ifstmt 			: IF expr THEN Slist ELSE Slist ENDIF	{$$ = createTree(0,IF_COND,NULL,BOOLTYPE,$2,$4,$6);}
				| IF expr THEN Slist ENDIF				{$$ = createTree(0,IF_COND,NULL,BOOLTYPE,$2,$4,NULL);}
				;

Whilestmt 		: WHILE expr DO Slist ENDWHILE			{$$ = createTree(0,WHILE_LOOP,NULL,BOOLTYPE,$2,NULL,$4);}

Dowhilestmt		: DO Slist WHILE expr					{$$ = createTree(0,DO_WHILE,NULL,BOOLTYPE,$4,NULL,$2);}

InputStmt 		: READ expr SEMICOLON					{$$ = createTree(0,READ_NODE,NULL,-1,$2,NULL,NULL);}
				;

OutputStmt 		: WRITE expr SEMICOLON					{$$ = createTree(0,WRITE_NODE,NULL,-1,$2,NULL,NULL);}
				;

AsgStmt 		: expr ASSIGN_OP expr SEMICOLON			{$$ = createTree(0,ASSIGN_NODE,NULL,-1,$1,NULL,$3);}
				;
%%

int yyerror(char const *s)
{
    printf("yyerror %s",yytext);
}


int main(void) {

	fout = fopen("sym_table.xsm","w");
	fin = fopen("sym_table.cod","r");
	yyin = fin;


	yyparse();

	return 0;
}

/*to do:
2 dimensional arrays(memory out of bounds not checked)
pointer var
modulo operator*/