%{
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	int yyerror(char const*);
	#include "inherit.c"
	int yylex(void);
	extern FILE* yyin;
	extern char* yytext;
	
	Typetable *type_enc,*temp_type, *cur_type_ptr;	//typetable entries
	int cur_type_size = 0, cur_field_index = 0;
	char* cur_type = NULL;
	
	Par_List* plistheader = NULL;
	LocSymTable* LocSymTableHeader = NULL;
	Fieldlist* temp_field = NULL;

	Classtable *ClassTableHeader = NULL, *cur_class = NULL,*cur_class_enc = NULL;
	Memberfunclist* fun_ptr;

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
%type <no> expr NUM program Slist Stmt InputStmt OutputStmt AsgStmt Ifstmt Whilestmt Dowhilestmt ID STRING ADDR GDeclBlock FDefBlock MainBlock Param Type ClassDef ClassDefBlock ClassDefList
%type <no> INT STR VOID NUL BOOL Fdecl DeclList Decl Fdef FunCallStmt ArgList ReturnStmt TypeDefBlock TypeDefList TypeDef Field UDtype MethodDefns SELF FieldFunction
%type <loc> LdeclBlock LDecList
%type <plist> ParamList
%type <flist> FieldDecl FieldDeclList
%token NUM PLUS MINUS MUL DIV MOD END ID READ WRITE ASSIGN_OP SEMICOLON START INT STR VOID NUL BOOL DECL ENDDECL STRING ADDR MAIN TYPE ENDTYPE EXTENDS
%token IF THEN ELSE ENDIF DO WHILE ENDWHILE LT GE GT LE EQ NEQ BREAK CONTINUE RETURN HEAPSET ALLOC FREE BREAKPOINT CLASS ENDCLASS NEW DELETE SELF
%right ASSIGN_OP
%left LT GT LE GE EQ NE
%left PLUS MINUS
%left MUL DIV MOD

%%

program 		: TypeDefBlock ClassDefBlock GDeclBlock FDefBlock MainBlock			{XsmHeader();
																					 ASTnode = createTree(0,CONNECT_NODE,NULL,-1,$4,NULL,$5,NULL);
																					 $$ = createTree(0,CONNECT_NODE,NULL,-1,$2,NULL,ASTnode,NULL);
																					 CodeGen($$,fout);}
				| TypeDefBlock ClassDefBlock GDeclBlock MainBlock					{XsmHeader();
																					 $$ = createTree(0,CONNECT_NODE,NULL,-1,$2,NULL,$4,NULL);
																					 CodeGen($$,fout);}
				| TypeDefBlock ClassDefBlock MainBlock								{XsmHeader();
																					 $$ = createTree(0,CONNECT_NODE,NULL,-1,$2,NULL,$3,NULL);
																					 CodeGen($$,fout);}
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
															if(cur_type_size >8){
																printf("memory out of bounds-%d\n",cur_type_size);
																exit(1);
															}
															
															cur_type_ptr = TInstall($1->varname,cur_type_size,$3);

															//all NULL field types are of type UDtype
															temp_field = cur_type_ptr->fields;
															while(temp_field != NULL){
																if(temp_field->type == NULL)
																	temp_field->type = cur_type_ptr;
																temp_field = temp_field->next;
															}

															cur_type_size = 0;
															cur_field_index = 0;
															cur_type = NULL;
															}
              	;

UDtype			: ID										{$$ = $1;cur_type = $1->varname;
															 if(TLookup($1->varname) != NULL){
																 printf("redeclaration of type %s\n",$1->varname);
																 exit(1);
															 }
															}

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
															 $$->Ctype = NULL;
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
															 //if the type is not a previously user defined strucutre
															 if(type_enc == NULL){

															 	//if its not a class type
																cur_class_enc = CLookup($1->varname);
															 	if( cur_class_enc == NULL){
															 		
																	 //if its not the current structure getting defined
															 		if(strcmp($1->varname,cur_type) != 0){
															 			printf("undeclared data type %s\n",$1->varname);
															 			exit(1);
															 		}
															 	}
															 }
															 else{
															 	cur_class_enc = NULL;
															 }
															 cur_type_size += 1;
															}
				;

ClassDefBlock 	: CLASS ClassDefList ENDCLASS				{$$ = $2;}
                |											{$$ = NULL;}
				;

ClassDefList 	: ClassDefList ClassDef						{$$ = createTree(0,CONNECT_NODE,NULL,-1,$2,NULL,$1,NULL);}
               	| ClassDef									{$$ = $1;}
				;

ClassDef      	: Cname '{'
							DECL 
								Fieldlists 
								MethodDecl 
							ENDDECL 
							
							MethodDefns 
						'}'									{
															 $$ = createTree(0,CLASS_DEF_LIST_NODE,cur_class->Name,-1,$7,NULL,NULL,NULL);
															 cur_class = NULL;
															}
				;

Cname         	: ID       									{ if(CLookup($1->varname) != NULL){
																  printf("redeclaration of data type %s\n",$1->varname);
																  exit(1);
															  }
															  ClassTableHeader = CInstall($1->varname,NULL); //check if parent class present later on
															  cur_class = CLookup($1->varname);
															}
				| ID EXTENDS ID								{
															 if(CLookup($1->varname) != NULL || CLookup($3->varname) == NULL){
																  printf("invalid declaration of data type %s\n",$1->varname);
																  exit(1);
															  }
															 ClassTableHeader = CInstall($1->varname,$3->varname);
															 cur_class = CLookup($1->varname);
															
															 //copy all member fields
															 temp_field = CLookup($3->varname)->memfieldptr;
															 while(temp_field != NULL){
																 Class_Finstall(cur_class,temp_field->type->name,temp_field->name);
																 temp_field = temp_field->next;
															 }
															// function copied during installation
															/*fun_ptr = CLookup($1->varname)->memfunptr;
															while(fun_ptr != NULL){
																printf("%s//////////////////\n",fun_ptr->Name);
																fun_ptr = fun_ptr->next;
															}*/
															}
				;

Fieldlists 		: Fieldlists Fld
				|
				;

Fld         	: Type ID SEMICOLON							{ Class_Finstall(cur_class,$1->varname,$2->varname);
															//it reports error if type is not declared before
															}
				;

MethodDecl 		: MethodDecl MDecl 		
	   			| MDecl 
				;

MDecl      		: Type ID '(' ParamList ')' SEMICOLON 		{
															 if(CLookup($1->varname) != NULL){
																 printf("method %s returns class\n");
																 exit(1);
															 }
															 
															 //no function overloading but overriding
															 if(Class_Mlookup(cur_class,$2->varname) != NULL){
																 FunSignatureCheck($2->varname,$4,TLookup($1->varname),cur_class);
																 printf("overriding of fun %s in %s\n",$2->varname,cur_class->Name);
															 }
															 Class_Minstall(cur_class,$2->varname,TLookup($1->varname),$4);
															 plistheader = NULL;
															 LocSymTableHeader = NULL;
															} 
				;

MethodDefns 	: MethodDefns Fdef							{$$ = createTree(0,CONNECT_NODE,NULL,-1,$2,NULL,$1,NULL);}
            	| Fdef										{$$ = $1;}
				;

FieldFunction 	: SELF '.' ID '(' ArgList ')'				{
															 $3->left = $5;
															 $$ = createTree(0,CLASS_FUN_CALL_NODE,"SELF_NODE",-1,NULL,NULL,NULL,$3);
															 //FunSignatureCheck is performed in createTree() itself
															}
				| SELF '.' ID '(' ')'						{
															 $$ = createTree(0,CLASS_FUN_CALL_NODE,"SELF_NODE",-1,NULL,NULL,NULL,$3);
															 //FunSignatureCheck is performed in createTree() itself
															}
                | ID '.' ID '(' ArgList ')'  				{//This will not occur inside a class
															 if(cur_class != NULL){
																 printf("invalid function call\n");
																 exit(1);
															 }
															 else{
															 	 $1->field = $3;
																 $3->left = $5;
																 $$ = createTree(0,CLASS_FUN_CALL_NODE,"NON_SELF_NODE",-1,NULL,NULL,NULL,$1);
																//FunSignatureCheck is performed in createTree() itself
															 }
															}
				| ID '.' ID '(' ')'							{
															 //This will not occur inside a class
															 if(cur_class != NULL){
																 printf("invalid function call\n");
																 exit(1);
															 }
															 else{
															 	 $1->field = $3;
																 $$ = createTree(0,CLASS_FUN_CALL_NODE,"NON_SELF_NODE",-1,NULL,NULL,NULL,$1);
																 //FunSignatureCheck is performed in createTree() itself
															 }
															}
                | Field '.' ID '(' ArgList ')'             	{
															 //if field starts with self then its self node else non self node
															 ASTnode = $1;
															 while($1->field != NULL)
															 	$1 = $1->field;
															 $1->field = $3;
															 $3->left = $5;
															 $$ = createTree(0,CLASS_FUN_CALL_NODE,ASTnode->varname,-1,NULL,NULL,NULL,ASTnode->field);
															}
				| Field '.' ID '(' ')'						{
															 ASTnode = $1;
															 while($1->field != NULL)
															 	$1 = $1->field;
															 $1->field = $3;
															 $$ = createTree(0,CLASS_FUN_CALL_NODE,ASTnode->varname,-1,NULL,NULL,NULL,ASTnode->field);
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

	

VarList 		: VarList ',' ID '[' NUM ']' '[' NUM ']'	{Install(($3)->varname,type_enc,NULL,($5)->val,$8->val,NULL);}
				| VarList ',' ID '[' NUM ']'				{Install(($3)->varname,type_enc,NULL,($5)->val,1,NULL);}
				| VarList ',' ID 							{Install(($3)->varname,type_enc,NULL,1,1,NULL);}
				| ID '[' NUM ']' '[' NUM ']'				{Install(($1)->varname,type_enc,NULL,($3)->val,($6)->val,NULL);}
				| ID '[' NUM ']'							{Install(($1)->varname,type_enc,NULL,($3)->val,1,NULL);}
				| ID										{Install(($1)->varname,type_enc,cur_class_enc,1,1,NULL);}
				;	

Field 			: Field '.' ID 								{printf("reduce Field\n");
															 $$ = $1;
															 while($1->field != NULL){
																 $1 = $1->field;
															 }
															 $1->field = $3;
															 $$->optype = $3->optype;
															}
				| ID '.' ID									{
															 if(cur_class != NULL){
																 //in fun def function inside class fun can access only local variables without self
																 if(LSTLookup(LocSymTableHeader,$1->varname) == NULL){
																	 printf("variable %s not accessible\n",$1->varname);
																	 exit(1);
																 }
																$$ = $1;
																$1->nodetype = FIELD;
																$$->optype = $3->optype; 
															 }
															 else{
																 //it occurs in fun def outside class def
																 //if $1->varname is class variable then its declared in global ST only
																 gst_ptr = Lookup($1->varname);
																 if(gst_ptr != NULL && gst_ptr->Ctype != NULL){
																	//we cannot access member fields outside class so if $1 is class type then outside class it can only call a member function
																	$$ = createTree(0,CLASS_FUN_CALL_NODE,"NON_SELF_NODE",-1,NULL,NULL,NULL,$1);
																	$1->optype = $3->optype;
																 }
																 else{
																	 $$ = $1;
																 	 $1->nodetype = FIELD;
																	 $$->optype = $3->optype; 
																 }
															 }
															 $1->field = $3;
															} 
				| SELF '.' ID								{
															 if(cur_class == NULL){
																 printf("invalid function call\n");
																 exit(1);
															 }
															 $$ = createTree(0,CLASS_FIELD,"SELF_NODE",-1,NULL,NULL,NULL,$3);
															}
				;

FDefBlock 	: FDefBlock Fdef								{$$ = createTree(0,CONNECT_NODE,NULL,-1,$2,NULL,$1,NULL);}
			| Fdef											{$$ = $1;}
			;

Fdef 		: Type ID '(' ParamList ')' 
				'{' 
					LdeclBlock 
					START
					Slist 
					ReturnStmt
					END
				'}'											{temp_type = TLookup(($1)->varname);
															 if(cur_class != NULL){
															 	$2->nodetype = CLASS_FUN_DEF_NODE;
															 	LocSymTableHeader = AddtoLocSymTable(LocSymTableHeader,"self",NULL,1);
																$7 = LocSymTableHeader;

															 }
															 else
															 	$2->nodetype = FUN_DEF_NODE;

															 $2->optype = $1->val;
															 $2->arglist = $4;
															 $2->lst_ptr = $7;			//includes arguments as well as local declarations
															 $2->left = $9;
															 $2->right = $10;
															 //return type must not be class 
															 if(CLookup($1->varname) != NULL){
																 printf("return type of %s() is of type <class>\n",$2->varname);
																 exit(1);
															 }
															 //if cur_class is NULL then fun is not a field
															 FunSignatureCheck($2->varname,$4,temp_type,cur_class);
															 if(GetType($10->left,LocSymTableHeader) != temp_type){
																 printf("return type mismatch in %s()\n",$2->varname);
																 exit(1);
															 }
															 $$ = $2;
															 plistheader = NULL;
															 LocSymTableHeader = NULL;
															}
			| Type ID '(' ParamList ')' 
				'{' 
					LdeclBlock
					START 
					ReturnStmt
					END
				'}'											{temp_type = TLookup(($1)->varname);
															 if(cur_class != NULL){
															 	LocSymTableHeader = AddtoLocSymTable(LocSymTableHeader,"self",NULL,1);
															 	$2->nodetype = CLASS_FUN_DEF_NODE;
																$7 = LocSymTableHeader;
															 }
															 else
															 	$2->nodetype = FUN_DEF_NODE;

															 $2->optype = $1->val;
															 $2->arglist = $4;
															 $2->lst_ptr = $7;			//includes arguments as well as local declarations
															 $2->left = NULL;
															 $2->right = $9;
															 if(CLookup($1->varname) != NULL){
																 printf("return type of %s() is of type <class>\n",$2->varname);
																 exit(1);
															 }
															 FunSignatureCheck($2->varname,$4,temp_type,cur_class);
															 if(GetType($9->left,LocSymTableHeader) != temp_type){
																 printf("return type mismatch in %s()\n",$2->varname);
																 exit(1);
															 }
															 $$ = $2;
															 plistheader = NULL;
															 LocSymTableHeader = NULL;
															}
			;
Fdecl		: Type ID '(' ParamList ')' SEMICOLON			{temp_type = TLookup(($1)->varname);
															Install($2->varname,temp_type,NULL,1,1,plistheader);
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

LDecl 		: Type IdList SEMICOLON							{if(CLookup($1->varname) != NULL){
																printf("class variable cannot be local\n");
																exit(1);
															 }
															}
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
					START
					Slist 
					ReturnStmt
					END
				'}'										{temp_type = TLookup("INT");
														$$ = createTree(0,MAIN_NODE,NULL,INTTYPE,$8,NULL,$9,NULL);
														$$->lst_ptr = $6;
														$$->left = $8;
														$$->right = $9;
														if(GetType($9->left,LocSymTableHeader) != temp_type){
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
														 FunSignatureCheck($1->varname,NULL,NULL,NULL);
														 plistheader = NULL;
														 $$ = $1;
														}
				| ID '(' ArgList ')' 					{
														 $1->nodetype = FUN_CALL_NODE;
														 FunSignatureCheck($1->varname,plistheader,NULL,NULL);
														 plistheader = NULL;
														 $1->left = $3;
														 $$ = $1;
														}
				| ALLOC '(' ')'							{$$ = createTree(0,ALLOC_NODE,"ALLOC",-1,NULL,NULL,NULL,NULL);}
				| FieldFunction							{$$ = $1;}
				| NEW '(' ID ')'						{
														 if(CLookup($3->varname) != NULL){
														 	$$ = createTree(0,NEW_NODE,"NEW",-1,NULL,NULL,NULL,$3);
														 }
														 else{
															 printf("invalid variable used for new operation\n");
															 exit(1);
														 }
														}
				| DELETE '(' ID ')'					{
														 gst_ptr = Lookup($3->varname);
														 if(gst_ptr != NULL && gst_ptr->Ctype != NULL){
														 	$$ = createTree(0,DELETE_NODE,"DELETE",-1,$3,NULL,NULL,NULL);
														 }
														 else{
															 printf("invalid variable used for delete operation\n");
															 exit(1);
														 }
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
														 FunSignatureCheck($1->varname,NULL,NULL,NULL);
														 plistheader = NULL;
														}
				| ID '(' ArgList ')' SEMICOLON			{
														 $1->nodetype = FUN_CALL_NODE;
														 $1->left = $3;
														 FunSignatureCheck($1->varname,plistheader,NULL,NULL);
														 plistheader = NULL;
														}
				;
	
ReturnStmt		: RETURN  expr  SEMICOLON			{ 
														 $$ = createTree(0,RETURN_NODE,NULL,-1,$2,NULL,NULL,NULL);
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

	fout = fopen("inherit.xsm","w");
	fin = fopen("test2.cod","r");
	yyin = fin;
	TypeTableCreate();									//initialise type table with primitive types

	yyparse();											//how to allocate user defined DS and how to bind them if one has another in it (in heap)

	return 0;
}
