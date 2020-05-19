%{
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	int yyerror(char const*);
	#include "addfun.c"
	int yylex(void);
	extern FILE* yyin;
	extern char* yytext;
	int type_enc;
	Par_List* plistheader = NULL;
	LocSymTable* LocSymTableHeader = NULL;
	tnode* ASTnode = NULL;
	gst_entry* gst_ptr;
	int fun_type;	// to keep track of function whose AST is being built not yet implimented also type check

%}

%union{
	struct tnode *no;
	struct LSymbol *loc;
	struct ParamList *plist;
}
%type <no> expr NUM program Slist Stmt InputStmt OutputStmt AsgStmt Ifstmt Whilestmt Dowhilestmt ID STRING ADDR GDeclBlock FDefBlock MainBlock Param Type
%type <no> INT STR Fdecl DeclList Decl Fdef FunCallStmt ArgList ReturnStmt
%type <loc> LdeclBlock LDecList
%type <plist> ParamList
%token NUM PLUS MINUS MUL DIV MOD END ID READ WRITE ASSIGN_OP SEMICOLON START INT STR DECL ENDDECL STRING ADDR MAIN 
%token IF THEN ELSE ENDIF DO WHILE ENDWHILE LT GE GT LE EQ NEQ BREAK CONTINUE RETURN
%right ASSIGN_OP
%left LT GT LE GE EQ NE
%left PLUS MINUS
%left MUL DIV MOD

%%

program 		: GDeclBlock FDefBlock MainBlock			{XsmHeader();$$ = createTree(0,CONNECT_NODE,NULL,-1,$2,NULL,$3);CodeGen($$,fout);}
				| GDeclBlock MainBlock						{XsmHeader();$$ = $2;CodeGen($$,fout);}
				| MainBlock									{XsmHeader();$$ = $1;CodeGen($1,fout);}
				;	

GDeclBlock		: DECL DeclList ENDDECL 					{$$ = $2;}
				| DECL ENDDECL								{}
				;	

DeclList 		: DeclList Decl 							{$$ = createTree(0,CONNECT_NODE,NULL,-1,$2,NULL,$1);}
				| Decl										{$$ = $1;}
				;	

Decl 			: Type VarList SEMICOLON					{}
				| Fdecl										{}
				;	

Type 			: INT 										{type_enc = INTTYPE;$$ = $1;}
				| STR										{type_enc = STRTYPE;$$ = $1;}
				;	

VarList 		: VarList ',' ID '[' NUM ']' '[' NUM ']'	{Install(($3)->varname,type_enc,($5)->val,$8->val,NULL);}
				| VarList ',' ID '[' NUM ']'				{Install(($3)->varname,type_enc,($5)->val,1,NULL);}
				| VarList ',' ID 							{Install(($3)->varname,type_enc,1,1,NULL);}
				| VarList ',' MUL ID				{	
															 if(type_enc == INTTYPE)
																Install(($4)->varname,INTPTR,1,1,NULL);
															 else
																Install(($4)->varname,STRPTR,1,1,NULL);
															}
				| ID '[' NUM ']' '[' NUM ']'				{Install(($1)->varname,type_enc,($3)->val,($6)->val,NULL);}
				| ID '[' NUM ']'							{Install(($1)->varname,type_enc,($3)->val,1,NULL);}
				| ID										{Install(($1)->varname,type_enc,1,1,NULL);}
				| MUL ID									{
															 if(type_enc == INTTYPE)
																Install(($2)->varname,INTPTR,1,1,NULL);
															 else
																Install(($2)->varname,STRPTR,1,1,NULL);
															}
				;	


FDefBlock 	: FDefBlock Fdef								{$$ = createTree(0,CONNECT_NODE,NULL,-1,$2,NULL,$1);}
			| Fdef											{$$ = $1;}
			;

Fdef 		: Type ID '(' ParamList ')' 
				'{' 
					LdeclBlock 
					Slist 
					ReturnStmt
				'}'											{$2->nodetype = FUN_DEF_NODE;
															 $2->optype = $1->val;
															 $2->arglist = $4;
															 $2->lst_ptr = $7;			//includes arguments as well as local declarations
															 $2->left = $8;
															 $2->right = $9;
															 FunSignatureCheck($2->varname,$4,$1->val);
															 if(GetType($9->left,LocSymTableHeader) != $1->val){
																 printf("return type mismatch\n");
																 exit(1);
															 }
															 $$ = $2;
															 plistheader = NULL;
															 LocSymTableHeader = NULL;
															}
			| Type MUL ID '(' ParamList ')'
			 	'{' 
			 		LdeclBlock 
					Slist 
					ReturnStmt
				'}'											{
															 $3->nodetype = FUN_DEF_NODE;
															 $3->arglist = $5;
															 $3->lst_ptr = $8;
															 $3->left = $9;
															 $3->right = $10;
															 FunSignatureCheck($3->varname,$5,$1->val+2);
															 if(GetType($10->left,LocSymTableHeader) != $1->val){
																 printf("return type mismatch\n");
																 exit(1);
															 }
															 $$ = $3;
															 plistheader = NULL;
															 LocSymTableHeader = NULL;
															}
			;

Fdecl		: Type ID '(' ParamList ')' SEMICOLON			{Install($2->varname,$1->val,1,1,plistheader);
															plistheader = NULL;
															LocSymTableHeader = NULL;}
			| Type MUL ID '(' ParamList ')' SEMICOLON		{Install($3->varname,($1->val+2),1,1,plistheader);  //inttype = 1 and intptrtype = 3;
															plistheader = NULL;
															LocSymTableHeader = NULL;
															}
			;

ParamList 	: ParamList ',' Param 							{plistheader = AddToParamList(plistheader,type_enc,$3->varname);
															//check if 2 arguments have same variable name
															if(LSTLookup(LocSymTableHeader,$3->varname) != NULL){
																printf("multiple declaration of %s\n",$3->varname);
																exit(1);
															}
															LocSymTableHeader = AddtoLocSymTable(LocSymTableHeader,$3->varname,type_enc,1);
															$$ = plistheader;}
			| Param											{plistheader = AddToParamList(plistheader,type_enc,$1->varname);
															LocSymTableHeader = AddtoLocSymTable(LocSymTableHeader,$1->varname,type_enc,1);
															$$ = plistheader;}
			| 												{$$ = plistheader;}
			;

Param 		: Type ID										{$$ = $2;}
			;

LdeclBlock 	: DECL LDecList ENDDECL 						{
															 $$ = LocSymTableHeader;
															 // LdeclBlock is assingned to tnode->lst_ptr later,so LdeclBlock is of type LocSymTable
															}
			| DECL ENDDECL									{$$ = LocSymTableHeader;}
			;

LDecList 	: LDecList LDecl 								{}
			| LDecl											{}
			;

LDecl 		: Type IdList SEMICOLON							{}
			;
IdList 		: IdList ',' ID 								{
															if(LSTLookup(LocSymTableHeader,$3->varname) != NULL){
																printf("multiple declaration of %s\n",$3->varname);
																exit(1);
															}
															LocSymTableHeader = AddtoLocSymTable(LocSymTableHeader,$3->varname,type_enc,0);}
			| ID											{
															//check if arguments already has variable of same name(arg already added in ParamList)
															if(LSTLookup(LocSymTableHeader,$1->varname) != NULL){
																printf("multiple declaration of %s\n",$1->varname);
																exit(1);
															}
															LocSymTableHeader = AddtoLocSymTable(LocSymTableHeader,$1->varname,type_enc,0);}
			;

MainBlock 	: INT MAIN '(' ')' 
				'{' 
					LdeclBlock 
					Slist 
					ReturnStmt
				'}'										{
														$$ = createTree(0,MAIN_NODE,NULL,INTTYPE,$7,NULL,$8);
														$$->lst_ptr = $6;
														$$->left = $7;
														$$->right = $8;
														if(GetType($8->left,LocSymTableHeader) != $1->val){
															printf("return type mismatch\n");
															exit(1);
														}
														LocSymTableHeader = NULL;
														plistheader = NULL;
														printf("type check done\n");
                                						}
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
				| ID '[' expr ']' '[' expr ']'			{$1->left = $3;$1->right = $6;$$ = $1;}
				| ID '[' expr ']'						{$1->left = $3;$$ = $1;}
				| ID									{$$ = $1;}
	 			| NUM									{$$ = $1;}
				| STRING								{$$ = $1;}
				| ADDR ID								{$2->nodetype = ADDRESS_NODE; $$ = $2;}
				| MUL ID								{$2->nodetype = POINTER_NODE;$$ = $2;}
				| ID '(' ')' 							{
														 $1->nodetype = FUN_CALL_NODE;
														 FunSignatureCheck($1->varname,NULL,-1);
														 plistheader = NULL;
														 $$ = $1;
														}
				| ID '(' ArgList ')' 					{
														 $1->nodetype = FUN_CALL_NODE;
														 FunSignatureCheck($1->varname,plistheader,-1);
														 plistheader = NULL;
														 $1->left = $3;
														 $$ = $1;
														}
				;

ArgList 		: ArgList ',' expr 						{plistheader = $1->arglist;
														 plistheader = AddToParamList(plistheader,GetType($3,LocSymTableHeader),"varname");
														 ASTnode = $1;
														 while(ASTnode->mid != NULL){
														 	ASTnode = ASTnode->mid;
														 }
														 ASTnode->mid = $3;
														 $1->arglist = plistheader;
														 $$ = $1;
														}

				| expr									{plistheader = NULL;
														 plistheader = AddToParamList(plistheader,GetType($1,LocSymTableHeader),"varname");
														 $1->arglist = plistheader;
														 $$ = $1;
														}
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
				| FunCallStmt							{$$ = $1;}
				
				;

FunCallStmt		: ID '(' ')' SEMICOLON					{
														 $1->nodetype = FUN_CALL_NODE;
														 FunSignatureCheck($1->varname,NULL,-1);
														 plistheader = NULL;
														}
				| ID '(' ArgList ')' SEMICOLON			{
														 $1->nodetype = FUN_CALL_NODE;
														 $1->left = $3;
														 FunSignatureCheck($1->varname,plistheader,-1);
														 plistheader = NULL;
														}
				;
	
ReturnStmt		: RETURN '(' expr ')' SEMICOLON			{ 
														 $$ = createTree(0,RETURN_NODE,NULL,-1,$3,NULL,NULL);
														}


Ifstmt 			: IF expr THEN 
					Slist 
				  ELSE Slist 
				  ENDIF SEMICOLON 						{$$ = createTree(0,IF_COND,NULL,BOOLTYPE,$2,$4,$6);}
				| IF expr THEN
					Slist 
				  ENDIF SEMICOLON						{$$ = createTree(0,IF_COND,NULL,BOOLTYPE,$2,$4,NULL);}
				;

Whilestmt 		: WHILE expr DO Slist ENDWHILE SEMICOLON {$$ = createTree(0,WHILE_LOOP,NULL,BOOLTYPE,$2,NULL,$4);}

Dowhilestmt		: DO Slist WHILE expr SEMICOLON			{$$ = createTree(0,DO_WHILE,NULL,BOOLTYPE,$4,NULL,$2);}

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

	fout = fopen("addfun.xsm","w");
	fin = fopen("addfun.cod","r");
	yyin = fin;


	yyparse();

	return 0;
}
