%{
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	int yyerror(char const*);
	#include "dynmem.c"
	int yylex(void);
	extern FILE* yyin;
	extern char* yytext;
	
	Typetable *type_enc,*temp_type, *cur_type_ptr;	//typetable entries
	int cur_type_size = 0, cur_field_index = 0;
	char* cur_type = NULL;
	
	Par_List* plistheader = NULL;
	LocSymTable* LocSymTableHeader = NULL;
	Fieldlist* temp_field = NULL;

	tnode* ASTnode = NULL;
	gst_entry* gst_ptr;
	int fun_type;	// to keep track of function whose AST is being built not yet implimented also type check

%}

%union{
	struct tnode *no;
	struct LSymbol *loc;
	struct ParamList *plist;
	struct Fieldlist *flist;
}
%type <no> expr NUM program Slist Stmt InputStmt OutputStmt AsgStmt Ifstmt Whilestmt Dowhilestmt ID STRING ADDR GDeclBlock FDefBlock MainBlock Param Type
%type <no> INT STR VOID NUL BOOL Fdecl DeclList Decl Fdef FunCallStmt ArgList ReturnStmt TypeDefBlock TypeDefList TypeDef Field UDtype
%type <loc> LdeclBlock LDecList
%type <plist> ParamList
%type <flist> FieldDecl FieldDeclList
%token NUM PLUS MINUS MUL DIV MOD END ID READ WRITE ASSIGN_OP SEMICOLON START INT STR VOID NUL BOOL DECL ENDDECL STRING ADDR MAIN TYPE ENDTYPE
%token IF THEN ELSE ENDIF DO WHILE ENDWHILE LT GE GT LE EQ NEQ BREAK CONTINUE RETURN HEAPSET ALLOC FREE BREAKPOINT 
%right ASSIGN_OP
%left LT GT LE GE EQ NE
%left PLUS MINUS
%left MUL DIV MOD

%%

program 		: TypeDefBlock GDeclBlock FDefBlock MainBlock			{XsmHeader();$$ = createTree(0,CONNECT_NODE,NULL,-1,$3,NULL,$4,NULL);CodeGen($$,fout);}
				| TypeDefBlock GDeclBlock MainBlock						{XsmHeader();$$ = $3;CodeGen($$,fout);}
				| TypeDefBlock MainBlock								{XsmHeader();$$ = $2;CodeGen($2,fout);}
				;	

TypeDefBlock  	: TYPE TypeDefList ENDTYPE					{}
              	|                                           {}
              	;											

TypeDefList   	: TypeDefList TypeDef						{}
              	| TypeDef									{}
              	;

TypeDef       	: UDtype '{' 
						FieldDeclList 
					'}'   								{	
															if(cur_type_size >16){
																printf("memory out of bounds-%d\n",cur_type_size);
																exit(1);
															}
															cur_type_ptr = TInstall($1->varname,cur_type_size,$3);
															temp_field = cur_type_ptr->fields;
															while(temp_field != NULL){
																if(temp_field->type == NULL)
																	temp_field->type = cur_type_ptr;
																temp_field = temp_field->next;
															}
															cur_type_size = 0;
															cur_field_index = 0;
															}
              	;

UDtype			: ID										{$$ = $1;cur_type = $1->varname;}

FieldDeclList 	: FieldDeclList FieldDecl					{temp_field = $1;
															 while(temp_field->next != NULL)
															 	temp_field = temp_field->next;
															 temp_field->next = $2;
															 $$ = $1;

															}
              	| FieldDecl 								{$$ = $1;}
              	;

FieldDecl    	: Type ID SEMICOLON							{$$ = (Fieldlist*)malloc(sizeof(Fieldlist));
															 $$->name = $2->varname;
															 $$->fieldIndex = cur_field_index; cur_field_index += 1;
															 $$->type = type_enc;
															 $$->next = NULL;
															}
				;

Type 			: INT 										{$$ = $1;type_enc = TLookup("INT");cur_type_size += 1;}
				| STR										{$$ = $1;type_enc = TLookup("STR");cur_type_size += 1;}
				| BOOL										{$$ = $1;type_enc = TLookup("BOOL");cur_type_size += 1;}
				| NUL										{$$ = $1;type_enc = TLookup("NULL");cur_type_size += 1;}
				| VOID										{$$ = $1;type_enc = TLookup("VOID");cur_type_size += 1;}
				| ID										{	
																//type of user defined structures
																type_enc = TLookup($1->varname);
																if(type_enc == NULL){
																	if(strcmp($1->varname,cur_type) != 0){
																		printf("undeclared data type %s\n",$1->varname);
																		exit(1);
																	}
																	cur_type_size += 1;
																}
																else
																	cur_type_size += GetSize(type_enc);
															}
				;

GDeclBlock		: DECL DeclList ENDDECL 					{$$ = $2;}
				| DECL ENDDECL								{}
				;	

DeclList 		: DeclList Decl 							{$$ = createTree(0,CONNECT_NODE,NULL,-1,$2,NULL,$1,NULL);}
				| Decl										{$$ = $1;}
				;	

Decl 			: Type VarList SEMICOLON					{}
				| Fdecl										{}
				;	

	

VarList 		: VarList ',' ID '[' NUM ']' '[' NUM ']'	{Install(($3)->varname,type_enc,($5)->val,$8->val,NULL);}
				| VarList ',' ID '[' NUM ']'				{Install(($3)->varname,type_enc,($5)->val,1,NULL);}
				| VarList ',' ID 							{Install(($3)->varname,type_enc,1,1,NULL);}
				| ID '[' NUM ']' '[' NUM ']'				{Install(($1)->varname,type_enc,($3)->val,($6)->val,NULL);}
				| ID '[' NUM ']'							{Install(($1)->varname,type_enc,($3)->val,1,NULL);}
				| ID										{Install(($1)->varname,type_enc,1,1,NULL);}
				;	

Field 			: Field '.' ID 								{printf("reduce Field\n");
															 $$ = $1;
															 while($1->field != NULL){
																 $1 = $1->field;
															 }
															 $1->field = $3;
															 $1->nodetype = FIELD;
															 $$->optype = $3->optype;
															}
				| ID '.' ID									{$1->field = $3;
															 $1->nodetype = FIELD;
															 $$ = $1;
															 $$->optype = $3->optype; 
															} 
				;

FDefBlock 	: FDefBlock Fdef								{$$ = createTree(0,CONNECT_NODE,NULL,-1,$2,NULL,$1,NULL);}
			| Fdef											{$$ = $1;}
			;

Fdef 		: Type ID '(' ParamList ')' 
				'{' 
					LdeclBlock 
					Slist 
					ReturnStmt
				'}'											{temp_type = TLookup(($1)->varname);
															 $2->nodetype = FUN_DEF_NODE;
															 $2->optype = $1->val;
															 $2->arglist = $4;
															 $2->lst_ptr = $7;			//includes arguments as well as local declarations
															 $2->left = $8;
															 $2->right = $9;
															 FunSignatureCheck($2->varname,$4,temp_type);
															 if(GetType($9->left,LocSymTableHeader) != temp_type){
																 printf("return type mismatch\n");
																 exit(1);
															 }
															 $$ = $2;
															 plistheader = NULL;
															 LocSymTableHeader = NULL;
															}
		
			;

Fdecl		: Type ID '(' ParamList ')' SEMICOLON			{temp_type = TLookup(($1)->varname);
															Install($2->varname,temp_type,1,1,plistheader);
															plistheader = NULL;
															LocSymTableHeader = NULL;}
															
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
				'}'										{temp_type = TLookup("INT");
														$$ = createTree(0,MAIN_NODE,NULL,INTTYPE,$7,NULL,$8,NULL);
														$$->lst_ptr = $6;
														$$->left = $7;
														$$->right = $8;
														if(GetType($8->left,LocSymTableHeader) != temp_type){
															printf("return type mismatch\n");
															exit(1);
														}
														LocSymTableHeader = NULL;
														plistheader = NULL;
														printf("type check done\n");
                                						}
          	;


expr 			: expr PLUS expr						{$$ = createTree(0,PLUS_OP,0,INTTYPE,$1,NULL,$3,NULL);}
	 			| expr MINUS expr  						{$$ = createTree(0,MINUS_OP,0,INTTYPE,$1,NULL,$3,NULL);}
	 			| expr MUL expr							{$$ = createTree(0,MUL_OP,0,INTTYPE,$1,NULL,$3,NULL);}
	 			| expr DIV expr							{$$ = createTree(0,DIV_OP,0,INTTYPE,$1,NULL,$3,NULL);}
				| expr MOD expr							{$$ = createTree(0,MODULO_OP,0,INTTYPE,$1,NULL,$3,NULL);}	
	 			| '(' expr ')'							{$$ = $2;}
	 			| expr LT expr							{$$ = createTree(0,RELOP_LT,0,BOOLTYPE,$1,NULL,$3,NULL);}
				| expr GT expr							{$$ = createTree(0,RELOP_GT,0,BOOLTYPE,$1,NULL,$3,NULL);}
				| expr LE expr							{$$ = createTree(0,RELOP_LE,0,BOOLTYPE,$1,NULL,$3,NULL);}
				| expr GE expr							{$$ = createTree(0,RELOP_GE,0,BOOLTYPE,$1,NULL,$3,NULL);}	
				| expr NE expr							{$$ = createTree(0,RELOP_NE,0,BOOLTYPE,$1,NULL,$3,NULL);}
				| expr EQ expr							{$$ = createTree(0,RELOP_EQ,0,BOOLTYPE,$1,NULL,$3,NULL);}
				| ID '[' expr ']' '[' expr ']'			{$1->left = $3;$1->right = $6;$$ = $1;}
				| ID '[' expr ']'						{$1->left = $3;$$ = $1;}
				| ID									{$$ = $1;}
				| NUL									{$$ = $1;}
				| Field									{$$ = $1;}
	 			| NUM									{$$ = $1;}
				| STRING								{$$ = $1;}
				| ADDR ID								{$2->nodetype = ADDRESS_NODE; $$ = $2;}
				| MUL ID								{$2->nodetype = POINTER_NODE;$$ = $2;}
				| ID '(' ')' 							{
														 $1->nodetype = FUN_CALL_NODE;
														 FunSignatureCheck($1->varname,NULL,NULL);
														 plistheader = NULL;
														 $$ = $1;
														}
				| ID '(' ArgList ')' 					{
														 $1->nodetype = FUN_CALL_NODE;
														 FunSignatureCheck($1->varname,plistheader,NULL);
														 plistheader = NULL;
														 $1->left = $3;
														 $$ = $1;
														}
				| ALLOC '(' ')'							{$$ = createTree(0,ALLOC_NODE,NULL,-1,NULL,NULL,NULL,NULL);}
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


Slist 			: Slist Stmt 							{$$ = createTree(0,CONNECT_NODE,NULL,-1,$1,NULL,$2,NULL);}
				| Stmt									{$$ = $1;}
				;

Stmt 			: InputStmt 							{$$ = $1;}
				| OutputStmt 							{$$ = $1;}
				| AsgStmt								{$$ = $1;}
				| BREAK SEMICOLON						{$$ = createTree(0,BREAK_NODE,NULL,-1,NULL,NULL,NULL,NULL);}
				| CONTINUE SEMICOLON					{$$ = createTree(0,CONTINUE_NODE,NULL,-1,NULL,NULL,NULL,NULL);}
				| Ifstmt								{$$ = $1;}
				| Whilestmt								{$$ = $1;}
				| Dowhilestmt							{$$ = $1;}
				| FunCallStmt							{$$ = $1;}
				| HEAPSET '(' ')' SEMICOLON				{$$ = createTree(0,HEAPSET_NODE,NULL,-1,NULL,NULL,NULL,NULL);}
				| FREE '(' ID ')' SEMICOLON				{$$ = createTree(0,FREE_NODE,NULL,-1,$3,NULL,NULL,NULL);}
				| BREAKPOINT SEMICOLON					{$$ = createTree(0,BREAKPOINT_NODE,NULL,-1,NULL,NULL,NULL,NULL);}
				;

FunCallStmt		: ID '(' ')' SEMICOLON					{
														 $1->nodetype = FUN_CALL_NODE;
														 FunSignatureCheck($1->varname,NULL,NULL);
														 plistheader = NULL;
														}
				| ID '(' ArgList ')' SEMICOLON			{
														 $1->nodetype = FUN_CALL_NODE;
														 $1->left = $3;
														 FunSignatureCheck($1->varname,plistheader,NULL);
														 plistheader = NULL;
														}
				;
	
ReturnStmt		: RETURN '(' expr ')' SEMICOLON			{ 
														 $$ = createTree(0,RETURN_NODE,NULL,-1,$3,NULL,NULL,NULL);
														}
				;

Ifstmt 			: IF expr THEN 
					Slist 
				  ELSE Slist 
				  ENDIF SEMICOLON 						{$$ = createTree(0,IF_COND,NULL,BOOLTYPE,$2,$4,$6,NULL);}
				| IF expr THEN
					Slist 
				  ENDIF SEMICOLON						{$$ = createTree(0,IF_COND,NULL,BOOLTYPE,$2,$4,NULL,NULL);}
				;

Whilestmt 		: WHILE expr DO Slist ENDWHILE SEMICOLON {$$ = createTree(0,WHILE_LOOP,NULL,BOOLTYPE,$2,NULL,$4,NULL);}
				;

Dowhilestmt		: DO Slist WHILE expr SEMICOLON			{$$ = createTree(0,DO_WHILE,NULL,BOOLTYPE,$4,NULL,$2,NULL);}
				;

InputStmt 		: READ expr SEMICOLON					{$$ = createTree(0,READ_NODE,NULL,-1,$2,NULL,NULL,NULL);}
				;

OutputStmt 		: WRITE expr SEMICOLON					{$$ = createTree(0,WRITE_NODE,NULL,-1,$2,NULL,NULL,NULL);}
				;

AsgStmt 		: expr ASSIGN_OP expr SEMICOLON			{$$ = createTree(0,ASSIGN_NODE,NULL,-1,$1,NULL,$3,NULL);}
				;
%%

int yyerror(char const *s)
{
    printf("yyerror %s",yytext);
}


int main(void) {

	fout = fopen("dynmem.xsm","w");
	fin = fopen("ext_euclid.cod","r");
	yyin = fin;
	TypeTableCreate();									//initialise type table with primitive types

	yyparse();											//how to allocate user defined DS and how to bind them if one has another in it (in heap)

	return 0;
}
