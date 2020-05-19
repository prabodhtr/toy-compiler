#include "dynmem.h"
#include <stdio.h>
extern LocSymTable* LocSymTableHeader;
int heaplist[128];                                  //to keep track of heap allocation

int Ar = -1,label = -1,sp = 4096,FunLabel = -1;
int label_Ar[20],lab_count = 0,var_count = 0;       // to keep track of labels
int cur_fun_loc_var_count = 0;                      // to keep track of local var to be popped out
tnode* cur_fun_node;                                // to know the function being executed right now
Typetable* TypeTableHeader = NULL;                  //header to type table


void check(struct tnode*);                          // check array index and variable 

Par_List* AddToParamList(Par_List* plistheader,Typetable* type, char* name);
void FunSignatureCheck(char* funname,Par_List* paramlistheader,Typetable* ret_type);
Typetable* GetType(struct tnode*,LocSymTable* LSTHeader);  // to get type of a node

LocSymTable* AddtoLocSymTable(LocSymTable* LocSymTableHeader, char* name, Typetable* type,int ArgFlag);
LocSymTable* LSTLookup(LocSymTable* LSTHeader, char *name);

void TypeTableCreate();
Typetable* TLookup(char* name);
Typetable* TInstall(char* name, int size, Fieldlist* fields);
Fieldlist* FLookup(Typetable *type, char* name);
int GetSize(Typetable* type);

gst_entry* gst_header = NULL;

struct var
{
    char* name;
    int val;
}varAr[50];

struct tnode* createTree(int val, int nodetype, char* c, int optype, struct tnode *l,struct tnode *m, struct tnode *r,struct tnode *field){
    struct tnode* temp,*switch_temp;
    Typetable* type_l;
    Typetable* type_r;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->val = val;
    temp->optype = optype;
    temp->varname = c;
    temp->nodetype = nodetype;
    temp->field = field;
    temp->left = l;
    temp->mid = m;
    temp->right = r;
    printf("%d\n",temp->nodetype);


    //type checks are performed in CodeGen()
    switch(temp->nodetype){
        case PLUS_OP    :
        case MINUS_OP   :
        case MUL_OP     :
        case MODULO_OP  :
        case DIV_OP     :   type_l = GetType(temp->left,LocSymTableHeader);
                            type_r = GetType(temp->right,LocSymTableHeader);
                            if(type_r != type_l || strcmp(type_r->name,"INT") != 0 ){
                                printf("type mismatch\n");
                                exit(1);
                            }
                            break;

        case ASSIGN_NODE:   
                            type_l = GetType(temp->left,LocSymTableHeader);
                            if(temp->right->nodetype != ALLOC_NODE ){
                                type_r = GetType(temp->right,LocSymTableHeader);
                                if((type_r != type_l)&&(strcmp(type_r->name,"NULL") != 0)){
                                    printf("type mismatch\n");
                                    exit(1);
                                }
                            }
                                
                            printf("type check out!\n");
                            break;

        case WHILE_LOOP:
        case DO_WHILE:
        case IF_COND:       type_l = GetType(temp->left,LocSymTableHeader);
                            if(strcmp(type_l->name,"BOOL") != 0){
                                printf("type mismatch\n");
                                exit(1);
                            }
                            break;

        case RELOP_EQ   :
        case RELOP_GE   :   
        case RELOP_GT   :   
        case RELOP_LE   :
        case RELOP_LT   :
        case RELOP_NE   :   type_l = GetType(temp->left,LocSymTableHeader);
                            type_r = GetType(temp->right,LocSymTableHeader);
                            // here we can compare 2 strings
                            if((type_l != type_r || ((strcmp(type_l->name,"BOOL") == 0) || (strcmp(type_r->name,"BOOL") == 0))) && strcmp(type_r->name,"NULL") != 0){
                                printf("type mismatch\n");
                                exit(1);
                            }
                            break;

        case WRITE_NODE :   type_l = GetType(temp->left,LocSymTableHeader);
                            if(strcmp(type_l->name,"INT") != 0 && strcmp(type_l->name,"STR") != 0){
                                printf("type mismatch in write\n");
                                exit(1);
                            }
                            break;

        case READ_NODE  :   
                            if(temp->left->nodetype != VAR){
                                printf("type mismatch in read\n");
                                exit(1);
                            }
                            break;
    }
    return temp;
}

Typetable* GetType(struct tnode* t,LocSymTable* LocSymTableHeader){
    gst_entry* gst_ptr;
    LocSymTable* LST_ptr;
    Typetable* temp_type;
    Fieldlist* temp_field;
    switch(t->nodetype){
        case CONSTANT   :   if(t->optype == INTTYPE)
                                return TLookup("INT");
                            else
                                return TLookup("STR");
                            break;

        /*case POINTER_NODE:  LST_ptr = LSTLookup(LocSymTableHeader,t->varname);
                            if(LST_ptr == NULL){
                                gst_ptr = Lookup(t->varname);
                                if(gst_ptr == NULL){
                                    printf("undeclared variable %s\n",t->varname);
                                    exit(1);
                                }
                                if(gst_ptr->type == INTPTR)
                                    return INTTYPE;
                                else 
                                    return STRTYPE;
                            }
                            if(LST_ptr->type == INTPTR)
                                    return INTTYPE;
                                else 
                                    return STRTYPE;
                            break;
        */

        case VAR        :   LST_ptr = LSTLookup(LocSymTableHeader,t->varname);
                            if(LST_ptr == NULL){
                                gst_ptr = Lookup(t->varname);
                                if(gst_ptr == NULL){
                                    printf("undeclared variable %s\n",t->varname);
                                    exit(1);
                                }
                                return gst_ptr->type;
                            }
                            return LST_ptr->type;
                            break;

        case FIELD      :   printf("field type check in\n");
                            LST_ptr = LSTLookup(LocSymTableHeader,t->varname);
                            if(LST_ptr == NULL){
                                gst_ptr = Lookup(t->varname);
                                if(gst_ptr == NULL){
                                    printf("undeclared variable %s\n",t->varname);
                                    exit(1);
                                }
                                temp_type = gst_ptr->type;
                            }
                            else{
                                temp_type = LST_ptr->type;
                            }
                            t = t->field;

                            while(t->field != NULL){
                                printf("field type check out %s %s\n",t->varname,temp_type->name);
                                
                                temp_field = FLookup(temp_type,t->varname);
                                printf("field type check  %s %s\n",t->varname,temp_field->type);
                                if(temp_type == NULL){
                                    printf("undeclared field %s\n",t->varname);
                                    exit(1);
                                }
                                temp_type = temp_field->type;
                                printf("temp_type = %s\n",temp_type->name);
                                t = t->field;
                            }

                            temp_field = FLookup(temp_type,t->varname);
                            if(temp_field == NULL){
                                printf("undeclared member field %s\n",t->field->varname);
                                exit(1);
                            }
                            return temp_field->type;
                            printf("field type check out\n");
                            break;

        case PLUS_OP    :
        case MINUS_OP   :
        case MUL_OP     :
        case DIV_OP     :   
        case MODULO_OP  :   return TLookup("INT");             
                            break;

        case RELOP_EQ   :
        case RELOP_GE   :
        case RELOP_GT   :
        case RELOP_LE   :
        case RELOP_LT   :
        case RELOP_NE   :   return TLookup("BOOL");
                            break;
        case TYPE_NODE  :  return TLookup(t->varname);
                            break;

    /*    case ADDRESS_NODE:  LST_ptr = LSTLookup(LocSymTableHeader,t->varname);
                            if(LST_ptr == NULL){
                                gst_ptr = Lookup(t->varname);
                                if(gst_ptr == NULL){
                                    printf("undeclared variable %s\n",t->varname);
                                    exit(1);
                                }
                                if(gst_ptr->type == INTTYPE)
                                    return INTADDRTYPE;
                                else 
                                    return STRADDRTYPE;
                            }
                            if(LST_ptr->type == INTTYPE)
                                    return INTADDRTYPE;
                                else 
                                    return STRADDRTYPE;
                            break;
    */
        case FUN_CALL_NODE: gst_ptr = Lookup(t->varname);
                            if(gst_ptr == NULL){
                                printf("undeclared fn %s\n",t->varname);
                                exit(1);
                            }
                            return gst_ptr->type;
                            break;
    }
}

void check(struct tnode* t){
    gst_entry* gst_ptr;
    LocSymTable* LST_entry;
    Typetable *type_r,*type_l;

    LST_entry = LSTLookup(LocSymTableHeader,t->varname);
    if(LST_entry == NULL){
        gst_ptr = Lookup(t->varname);
        if(gst_ptr == NULL){
            printf("undeclared variable \"%s\"\n",t->varname);
            exit(1);
        }
    }
    if(t->left != NULL){
        type_l = GetType(t->left,LocSymTableHeader);
        //printf("type %d\n",type_l);
        if(strcmp(type_l->name,"INT") != 0){
            printf("invalid array index\n");
            exit(1);
        }
        if(t->right != NULL){
            type_r = GetType(t->right,LocSymTableHeader);
            if(strcmp(type_r->name,"INT") != 0){
                printf("invalid array index\n");
                exit(1);
            }
        }
    }
}

void TypeTableCreate(){
    Typetable* temp;
    TypeTableHeader = (struct Typetable*)malloc(sizeof(struct Typetable));
    temp = TypeTableHeader;
    temp->name = "INT";
    temp->size = 1;
    temp->fields = NULL;
    temp->next = (struct Typetable*)malloc(sizeof(struct Typetable));
    temp = temp->next;

    temp->name = "STR";
    temp->size = 1;
    temp->fields = NULL;
    temp->next = (struct Typetable*)malloc(sizeof(struct Typetable));
    temp = temp->next;

    temp->name = "BOOL";
    temp->size = 1;
    temp->fields = NULL;
    temp->next = (struct Typetable*)malloc(sizeof(struct Typetable));
    temp = temp->next;

    temp->name = "NULL";
    temp->size = 1;
    temp->fields = NULL;
    temp->next = (struct Typetable*)malloc(sizeof(struct Typetable));
    temp = temp->next;

    temp->name = "VOID";
    temp->size = 1;
    temp->fields = NULL;
    temp->next = NULL;
}

Typetable* TLookup(char* name){
    Typetable* ptr = TypeTableHeader;
    while(ptr != NULL){
        if(strcmp(ptr->name,name) == 0)
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
}

Typetable* TInstall(char* name, int size, Fieldlist* fields){
    //when typedef in found in program
    Typetable* ptr = TypeTableHeader;
    while(ptr->next != NULL){
        ptr = ptr->next;
    }
    ptr->next = (struct Typetable*)malloc(sizeof(struct Typetable));
    ptr = ptr->next;
    ptr->name = name;
    ptr->size = size;
    ptr->fields = fields;
    ptr->next = NULL;
    return ptr;
}

Fieldlist* FLookup(Typetable* type, char *name){
    Fieldlist* ptr = type->fields;
    while(ptr != NULL){
        if(strcmp(ptr->name,name) == 0)
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
}

int GetSize(Typetable* type){
    return type->size;
}

int GetVarIndex(char* name){
    int count = 0;
    while(count < var_count){
        if(strcmp(varAr[count].name, name) == 0)
            return count;
        count = count +1 ;
        }
}

Par_List* AddToParamList(Par_List* parlistheader,Typetable* type, char* name){
    Par_List* temp = parlistheader;
    Par_List* new = (Par_List*)malloc(sizeof(Par_List));
    new->name = name;
    new->type = type;
    new->next = NULL;
    if(temp == NULL){
        return new;
    }
    while(temp->next != NULL){
        temp = temp->next;
    }
    temp->next = new;
    return parlistheader;
}

LocSymTable* AddtoLocSymTable(LocSymTable* LocSymTableHeader, char* name, Typetable* type,int ArgFlag){
    LocSymTable* temp = LocSymTableHeader;
    LocSymTable* new = (LocSymTable*)malloc(sizeof(LocSymTable));
    new->name = name;
    new->type = type;
    new->Arg_flag = ArgFlag;
    new->next = NULL;
    if(temp == NULL){
        new->binding = -1;                                   //local declarations are binded to address relative to position of BP
        return new;
    }
    while(temp->next != NULL){
        temp = temp->next;;
    }
    
    temp->next = new;
    new->binding = temp->binding + 1;
    return LocSymTableHeader;
}

LocSymTable* LSTLookup(LocSymTable* LSTHeader,char* name){
    LocSymTable* temp = LSTHeader;
    int i = 0;
    while(temp != NULL){
        if(strcmp(temp->name,name) == 0){
            return temp;
        }
        i++;
        temp = temp->next;
    }
    return NULL;
}

void FunSignatureCheck(char* funname,Par_List* paramlistheader,Typetable* ret_type_chk_flag){
    gst_entry* gst_ptr;
    gst_ptr = Lookup(funname);
    if(gst_ptr == NULL){
        printf("function undeclared\n");
        exit(1);
    }
    if(ret_type_chk_flag != NULL){
        //FunSignatureCheck() not called from a fun call;return type needs to be checked
        if(ret_type_chk_flag != gst_ptr->type){
            printf("return type mismatch\n");
            exit(1);
        }
    }

    if(paramlistheader == NULL && gst_ptr->paramlist == NULL){
        return;
    }
    else if(paramlistheader == NULL || gst_ptr->paramlist == NULL){
        if(paramlistheader == NULL){printf("paramlistHeader NULL\n");}
        printf("type mismatch in fun(empty arg_list)\n");
        exit(1);
    }   
    else{
        Par_List* fun_parlist_header;
        fun_parlist_header = gst_ptr->paramlist;
        while(paramlistheader != NULL){
            if(fun_parlist_header == NULL){
                printf("type mismatch in fun(# arg)%s\n",paramlistheader->name);
                exit(1);
            }
            if(paramlistheader->type != fun_parlist_header->type){
                printf("type mismatch in fun\n");
                exit(1);
            }
            printf("%s-%s\n",paramlistheader->type->name,fun_parlist_header->type->name);
            paramlistheader = paramlistheader->next;
            fun_parlist_header = fun_parlist_header->next;
        }
        if(fun_parlist_header != NULL){
            printf("type mismatch in fun %s (# arg)\n",fun_parlist_header->name);
            exit(1);
        }
    }
}

struct Gsymbol* Lookup(char * name){
    gst_entry* temp = gst_header;
    while(temp != NULL){
        if(strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

int Install(char *name, Typetable* type, int size,int col_size,Par_List* paramlistheader){
    gst_entry* temp = gst_header;
    if(gst_header != NULL){
        while(temp->next != NULL){
            if(strcmp(temp->name, name) == 0){
                printf("Multiple declaration of variable %s\n",temp->name);  
                exit(1);                      //variable already exists
            }
            temp = temp->next;
        }
        if(strcmp(temp->name, name) == 0){
            printf("Multiple declaration of variable %s\n",temp->name);  
            exit(1);                      //variable already exists
        }
    }
    gst_entry* new = (gst_entry*)malloc(sizeof(gst_entry));
    new->name = name;
    new->type = type;
    new->size = size;
    new->col_size = col_size;
    new->paramlist = paramlistheader;
    new->next = NULL;
    if(gst_header == NULL){
        new->binding = 4096;
        new->flabel = 0;
        gst_header = new;
    }
    else{
        new->binding = temp->binding + (temp->size * temp->col_size);   //new location = start of prev loc + size of prev location
        new->flabel = temp->flabel + 1;                     
        temp->next = new;
    }
    printf("%s = %d\n",new->name,new->binding);

    return 1;
}

int GetReg(){
    if(Ar == 19){
        return -1;
    }
    return ++Ar;
}

int FreeReg(){
    if(Ar == 0){
        return -1;
    }
    Ar--;
}

int GetLabel(){
    
    return ++label;
}

int GetFunLabel(){
    return ++FunLabel;
}

void backup(){
    int i = Ar;
    while(i>0){
        fprintf(fout,"PUSH R%d\n",i);
        i--;
    }
}

void restore(){
    int i = 1;
    while(i<=Ar){
        fprintf(fout,"POP R%d\n",i);
        i++;
    }
}

int CodeGen(struct tnode* t,FILE* fout){
    int p,q,r,s,label_1,label_2;
    int i,j,cur_mem_reference;
    int binding_offset = 0;
    gst_entry* gst_ptr;
    tnode* ASTnode;
    Par_List* ArgList;
    LocSymTable* LSTentry,*LSTHeader;
    Fieldlist* temp_field;
    Typetable* temp_type;
    switch (t->nodetype){
        case CONSTANT   :   printf("cont_in\n");
                            p = GetReg();
                            if(p == -1)
                                return p;
                            if(t->optype == INTTYPE)
                                fprintf(fout,"MOV R%d,%d\n",p,t->val);
                            else
                                fprintf(fout,"MOV R%d,%s\n",p,t->varname);
                            printf("cont_out\n");
                            return p;
                            break;

        case VAR        :   printf("var_in\n");
                            LSTentry = LSTLookup(cur_fun_node->lst_ptr,t->varname);
                            if(LSTentry == NULL){
                                gst_ptr = Lookup(t->varname);
                                if(gst_ptr == NULL){
                                    printf("undeclared variable \"%s\"\n",t->varname);
                                    exit(1);
                                }
                            }

                            p = GetReg();
                            if(t->left != NULL){
                                fprintf(fout,"MOV R%d,%d\n",p,gst_ptr->binding);
                                q = CodeGen(t->left,fout);
                                if((t->left)->nodetype == VAR){
                                    s = GetReg();
                                    fprintf(fout,"MOV R%d,[R%d]\nMOV R%d,R%d\n",s,q,q,s);
                                    FreeReg();
                                }
                            
                                if(t->right != NULL){
                                    r = CodeGen(t->right,fout);
                                    s = GetReg();
                                    if((t->right)->nodetype == VAR){
                                        fprintf(fout,"MOV R%d,[R%d]\nMOV R%d,R%d\n",s,r,r,s);
                                    }
                                    //i * col_size + j
                                    fprintf(fout,"MOV R%d,%d\nMUL R%d,R%d\n",s,gst_ptr->col_size,q,s);  
                                    fprintf(fout,"ADD R%d,R%d\n",q,r);
                                    FreeReg();
                                    FreeReg();
                                }
                                fprintf(fout,"ADD R%d,R%d\n",p,q);
                                FreeReg();
                            }
                            else{
                                if(LSTentry == NULL)
                                    fprintf(fout,"MOV R%d,%d\n",p,gst_ptr->binding);
                                else{
                                    q = GetReg();
                                    printf("lstbindin = %d,%s\n",LSTentry->binding,LSTentry->name);
                                    fprintf(fout,"MOV R%d,BP\nMOV R%d,%d\n",p,q,LSTentry->binding);
                                    fprintf(fout,"ADD R%d,R%d\n",p,q);
                                    FreeReg();
                                }
                            }
                            printf("var_out\n");
                            return p;            //return the address of the location to which var is statically bounded
                            break;

        case FIELD      :   printf("field_in\n");
                            binding_offset = 0;
                            LSTentry = LSTLookup(cur_fun_node->lst_ptr,t->varname);
                            if(LSTentry == NULL){
                                gst_ptr = Lookup(t->varname);
                                if(gst_ptr == NULL){
                                    printf("undeclared variable \"%s\"\n",t->varname);
                                    exit(1);
                                }
                            }
                            if(LSTentry == NULL)
                            {
                                temp_type = gst_ptr->type;
                            }
                            else{
                                temp_type = LSTentry->type;
                            }
                            p = GetReg();

                            if(LSTentry == NULL){
                                fprintf(fout,"MOV R%d,%d\n",p,gst_ptr->binding);
                            }
                            else{
                                q = GetReg();
                                fprintf(fout,"MOV R%d,BP\nMOV R%d,%d\n",p,q,LSTentry->binding);
                                fprintf(fout,"ADD R%d,R%d\n",p,q);
                                FreeReg();
                            }
                            fprintf(fout,"MOV R%d,[R%d]\n",p,p);
                            t = t->field;
                            while(t != NULL){
                                //temp_field = FLookup(temp_type,t->varname);
                                temp_field = temp_type->fields;
                                while(strcmp(temp_field->name,t->varname) != 0){
                                    binding_offset += GetSize(temp_field->type);
                                    printf("cur_field_index = %d\n",temp_field->fieldIndex);
                                    temp_field = temp_field->next;
                                }
                                printf("binding_offset = %d\n",binding_offset);
                                temp_type = temp_field->type;
                                t = t->field;
                            }
                            if(binding_offset > 16){
                                printf("memory reference out of bounds\n");
                                exit(1);
                            }
                            q = GetReg();
                            fprintf(fout,"MOV R%d,%d\nADD R%d,R%d\n",q,binding_offset,p,q);
                            FreeReg();
                            printf("field_out\n");
                            return p;
                            break;

        case HEAPSET_NODE:  printf("heapset_in\n");
                            backup();
                            fprintf(fout,"MOV R1,\"Heapset\"\nPUSH R1\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\n");
                            restore();
                            fprintf(fout,"POP R0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\n");
                            for(i = 0;i < 127;i++){
                                heaplist[i] = i + 1;
                            }
                            heaplist[127] = -1;
                            printf("heapset_out\n");
                            break;

        case ALLOC_NODE :   printf("alloc_in\n");
                            p = GetReg();
                            fprintf(fout,"MOV R%d,16\n",p);
                            backup();
                            fprintf(fout,"MOV R1,\"Alloc\"\nPUSH R1\nPUSH R%d\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\n",p);
                            //get return value in R0
                            fprintf(fout,"POP R0\n");
                            fprintf(fout,"POP R1\nPOP R1\nPOP R1\nPOP R1\n");
                            restore();
                            fprintf(fout,"MOV R%d,R0\n",p);
                            //check for -1 if not

                            if(heaplist[0] == -1){
                                printf("memory out of bounds\n");
                            }
                            else{
                                cur_mem_reference = 1024 + heaplist[0]*8; 
                                heaplist[0] = heaplist[heaplist[0]];
                            }
                            printf("alloc_out\n");
                            return p;
                            break;

        case FREE_NODE  :   printf("free_in\n");
                            q = CodeGen(t->left,fout);
                            p = GetReg();
                            r = GetReg();
                            //moving 0 to the binding address of userdefined variable freed to avoid access before reallocation
                            fprintf(fout,"MOV R%d,[R%d]\nMOV [R%d],0\nMOV R%d,R%d\n",r,q,q,q,r);
                            FreeReg();
                            backup();
                            fprintf(fout,"MOV R1,\"Free\"\nPUSH R1\nPUSH R%d\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\n",q);
                            //get return value
                            fprintf(fout,"POP R0\n");
                            fprintf(fout,"POP R1\nPOP R1\nPOP R1\nPOP R1\n");
                            restore();
                            fprintf(fout,"MOV R%d,R0\n",p);
                            //check for -1 if not
                            
                            FreeReg();
                            FreeReg();
                            printf("free_out\n");
                            break;

        case POINTER_NODE:  gst_ptr = Lookup(t->varname);
                             p = GetReg();

                            if(gst_ptr == NULL){
                                printf("undeclared variable \"%s\"\n",t->varname);
                                exit(1);
                            }
                            q = GetReg();
                            fprintf(fout,"MOV R%d,%d\nMOV R%d,[R%d]\nMOV R%d,R%d\n",p,gst_ptr->binding,q,p,p,q);
                            FreeReg();
                            return p;
                            break;

        case ADDRESS_NODE:  gst_ptr = Lookup(t->varname);
                             p = GetReg();

                            if(gst_ptr == NULL){
                                printf("undeclared variable \"%s\"\n",t->varname);
                                exit(1);
                            }
                            fprintf(fout,"MOV R%d,%d\n",p,gst_ptr->binding);
                            return p;
                            break;

        case READ_NODE  :   p = CodeGen(t->left,fout);
                            if(p == 1){
                                q = GetReg();
                                fprintf(fout,"MOV R%d,R%d\n",q,p);
                                p = q;
                            }
                            XsmRead(p);
                            FreeReg();
                            FreeReg();
                            return p;
                            break;

        case WRITE_NODE:    p = CodeGen(t->left,fout);
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE || t->left->nodetype == FIELD){
                                q = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\nMOV R%d,R%d\n",q,p,p,q);
                                FreeReg();
                            }
                            if(p == 1){
                                q = GetReg();
                                fprintf(fout,"MOV R%d,R%d\n",q,p);
                                p = q;
                            }
                            XsmWrite(p);
                            FreeReg();
                            FreeReg();
                            return p;
                            break;

        case ASSIGN_NODE:   printf("assign_in\n");
                            p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE || t->right->nodetype == FIELD){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\nMOV R%d,R%d\n",r,q,q,r);
                                FreeReg();
                            }
                            fprintf(fout,"MOV [R%d],R%d\n",p,q);
                            FreeReg();
                            FreeReg();
                            printf("assign_out\n");
                            return p;
                            break;

        case TYPE_NODE   :   p = GetReg();
                            if(strcmp(t->varname,"NULL") == 0){
                                fprintf(fout,"MOV R%d,0\n",p);
                                return p;
                            }
                            FreeReg();
                            break;

        case PLUS_OP    :   //Match_int_optype(t);
                            p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE || t->left->nodetype == FIELD){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE || t->right->nodetype == FIELD){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"ADD R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;
                                                                
        case MINUS_OP   :   //Match_int_optype(t);
                            p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE || t->left->nodetype == FIELD){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE || t->right->nodetype == FIELD){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"SUB R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case MUL_OP     :   //Match_int_optype(t);
                            p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE || t->left->nodetype == FIELD){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE || t->right->nodetype == FIELD){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();                          
                            }
                            fprintf(fout,"MUL R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case DIV_OP     :   //Match_int_optype(t);
                            p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE || t->left->nodetype == FIELD){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE || t->right->nodetype == FIELD){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"DIV R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case MODULO_OP  :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE || t->left->nodetype == FIELD){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE || t->right->nodetype == FIELD){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"MOD R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;


        case RELOP_LT   :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE || t->left->nodetype == FIELD){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE || t->right->nodetype == FIELD){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"LT R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case RELOP_LE   :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE || t->left->nodetype == FIELD){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE || t->right->nodetype == FIELD){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"LE R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case RELOP_GT   :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE || t->left->nodetype == FIELD){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE || t->right->nodetype == FIELD){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"GT R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case RELOP_GE   :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE || t->left->nodetype == FIELD){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE || t->right->nodetype == FIELD){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"GE R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case RELOP_EQ   :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE || t->left->nodetype == FIELD){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE || t->right->nodetype == FIELD){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"EQ R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case RELOP_NE   :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE || t->left->nodetype == FIELD){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE || t->right->nodetype == FIELD){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"NE R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case FUN_CALL_NODE: //1.push registers to stack
                            j = GetReg();
                            FreeReg();
                            for(i = 0;i<j;i++){
                                fprintf(fout,"PUSH R%d\n",i);
                            }
                            //2.evaluate and push arguments to stack
                            ASTnode = t->left;
                            while(ASTnode != NULL){
                                p = CodeGen(ASTnode,fout);
                                if(ASTnode->nodetype == VAR || ASTnode->nodetype == FIELD){
                                    fprintf(fout,"MOV R%d,[R%d]\n",p,p);
                                }
                                fprintf(fout,"PUSH R%d\n",p);
                                FreeReg();
                                ASTnode = ASTnode->mid;
                            }
                            //3.push space for return value
                            fprintf(fout,"PUSH R0\n");
                            //4.call function label
                            gst_ptr = Lookup(t->varname);
                            i = gst_ptr->flabel;
                            fprintf(fout,"CALL F%d\n",i);

                            //after returning from callee
                            //1.store return value
                            p = GetReg();
                            fprintf(fout,"POP R%d\n",p);
                            //2.pop arguments and disgard them
                            ASTnode = t->left;
                            while(ASTnode != NULL){
                                fprintf(fout,"POP R0\n");
                                ASTnode = ASTnode->mid;
                            }
                            //3.restore registers
                            for(i = j;i > 0;i--){
                                fprintf(fout,"POP R%d\n",i-1);
                            }
                            return p;
                            break;

        case FUN_DEF_NODE:  cur_fun_node = t;
                            /*i = GetFunLabel();
                            0.add flabel to corresponding gst_entery and print label
                            gst_ptr->flabel = i;*/
                            gst_ptr = Lookup(t->varname);
                            fprintf(fout,"F%d:\n",gst_ptr->flabel);
                            printf("flabel = %d\n",gst_ptr->flabel);
                            //1.save BP
                            fprintf(fout,"PUSH BP\n");
                            //2.set BP to current SP
                            fprintf(fout,"MOV BP,SP\n");
                            //3.push space in stack for argument and local variables
                            
                            //3a.for each local var declarations push space and bind it relative to BP
                            printf("fun def name:%s\n",t->varname);

                            LSTentry = t->lst_ptr;
                            i = 0;
                            while(LSTentry != NULL){
                                if(LSTentry->Arg_flag == 0){
                                    fprintf(fout,"PUSH R0\n");
                                    LSTentry->binding = i + 1;
                                    i = i + 1;
                                }
                                LSTentry = LSTentry->next;
                            }
 
                            cur_fun_loc_var_count = i;
                            //3b.find number of arguments of function
                            ArgList = t->arglist;
                            i = 0;
                            while(ArgList != NULL){
                                //t->lst_ptr = AddtoLocSymTable(t->lst_ptr,ArgList->name,ArgList->type);
                                i = i + 1;
                                ArgList = ArgList->next;
                            }
                            //for each argument lookup in LST and bind to position

                            ArgList = t->arglist;
                            while(i > 0){
                                LSTentry = LSTLookup(t->lst_ptr,ArgList->name);
                                if(LSTentry == NULL){
                                    printf("loc variable not found in fun def\n");
                                    exit(1);
                                }
                                LSTentry->binding = -(i + 2);
                                ArgList = ArgList->next;
                                i--;
                            }
                            CodeGen(t->left,fout);
                            CodeGen(t->right,fout);
                            printf("fun def:%s() over\n",t->varname);

                            break;

        case RETURN_NODE:   //returning from the callee
                            //1.pop out local variables
                            while(cur_fun_loc_var_count != 0){
                                fprintf(fout,"POP R0\n");
                            cur_fun_loc_var_count--;
                            }
                            //to check if return type is same as that in declaration
                            /*i = GetType(t->left,LocSymTableHeader);
                            if(i != cur_fun_node->optype){
                                printf("return type mismatch\n");
                                exit(1);
                            }*/
                            //2.calculate return value and store it in BP-2
                            p = CodeGen(t->left,fout);
                            fprintf(fout,"SUB BP,2\n");
                            if(t->left->nodetype == VAR)
                                fprintf(fout,"MOV R%d,[R%d]\n",p,p);
                            fprintf(fout,"MOV [BP],R%d\n",p);
                            //3.set BP to old value of BP
                            fprintf(fout,"POP BP\n");
                            //4.execute RET
                            fprintf(fout,"RET\n");
                            break;

        case MAIN_NODE  :   cur_fun_node = t;
                            fprintf(fout,"MAIN:\n");
                            //1.save BP
                            fprintf(fout,"PUSH BP\n");
                            //2.set BP to current SP
                            fprintf(fout,"MOV BP,SP\n");
                            printf("fun def name:main()\n");
                            //3.push space in stack for argument and local variables
                            
                            //3a.for each local var declarations push space and bind it relative to BP
                            LSTentry = t->lst_ptr;
                            i = 0;
                            while(LSTentry != NULL){
                                fprintf(fout,"PUSH R0\n");
                                LSTentry->binding = i + 1;
                                i = i + 1;
                                LSTentry = LSTentry->next;
                            }
                            cur_fun_loc_var_count = i;
                            CodeGen(t->left,fout);
                            CodeGen(t->right,fout);
                            printf("fun main() over\n");
                            break;

        case CONNECT_NODE : CodeGen(t->left,fout);
                            CodeGen(t->right,fout);
                            break;

        case WHILE_LOOP :   label_1 = GetLabel();
                            label_2 = GetLabel();
                            label_Ar[lab_count] = label_1;
                            lab_count++;
                            label_Ar[lab_count] = label_2;
                            lab_count++;
                            fprintf(fout,"L%d:\n",label_1);
                            p = CodeGen(t->left,fout);
                            fprintf(fout,"JZ R%d,L%d\n",p,label_2);
                            q = CodeGen(t->right,fout);
                            fprintf(fout,"JMP L%d\n",label_1);
                            fprintf(fout,"L%d:\n",label_2);
                            lab_count = lab_count - 2;
                            break;

        case DO_WHILE   :   label_1 = GetLabel();
                            label_2 = GetLabel();
                            label_Ar[lab_count] = label_1;
                            lab_count++;
                            label_Ar[lab_count] = label_2;
                            lab_count++;
                            fprintf(fout,"L%d:\n",label_1);
                            q = CodeGen(t->right,fout);
                            p = CodeGen(t->left,fout);
                            fprintf(fout,"JZ R%d,L%d\n",p,label_2);
                            fprintf(fout,"JMP L%d\n",label_1);
                            fprintf(fout,"L%d:\n",label_2);
                            lab_count = lab_count - 2;
                            break;

        case IF_COND    :   label_1 = GetLabel();
                            label_2 = GetLabel();
                            p = CodeGen(t->left,fout);
                            fprintf(fout,"JZ R%d,L%d\n",p,label_1);
                            CodeGen(t->mid,fout);
                            if(t->right != NULL){
                                fprintf(fout,"JMP L%d\n",label_2);
                            }
                            fprintf(fout,"L%d:\n",label_1);
                            if((t->right) != NULL){
                                CodeGen(t->right,fout);
                                fprintf(fout,"L%d:\n",label_2);    
                            }
                            break;

        case BREAK_NODE :   if(lab_count != 0){
                                fprintf(fout,"JMP L%d\n",label_Ar[lab_count - 1]);
                                lab_count = lab_count - 2;
                                // pop 2 labels corresponding to the loop from stack
                            }
                            break;

        case CONTINUE_NODE: if(lab_count != 0){
                                fprintf(fout,"JMP L%d\n",label_Ar[lab_count - 2]);
                                lab_count = lab_count - 2;
                            }
                            break;
        
        case BREAKPOINT_NODE:fprintf(fout,"BRKP\n");
                            break;

        default         :   printf("invalid nodetype\n%d",t->nodetype);
                            exit(1);
    }
}

void XsmHeader(){
    gst_entry* temp = gst_header;
    while(temp->next != NULL)
        temp = temp->next;
    sp = temp->binding + (temp->size * temp->col_size);
    fprintf(fout,"0\n2056\n0\n0\n0\n0\n0\n0\nMOV SP,%d\nMOV BP,%d\n",sp,sp+1);
    fprintf(fout,"CALL MAIN\nINT 10\n");
}

void XsmWrite(int r_num){
    fprintf(fout,"MOV R1,\"Write\"\nPUSH R1\nMOV R1,-2\nPUSH R1\nPUSH R%d\nPUSH R1\nPUSH R1\nCALL 0\n",r_num);
	fprintf(fout,"POP R0\nPOP R1\nPOP R1\nPOP R1\nPOP R1\nBRKP\n");
}

void XsmRead(int reg_no){
    fprintf(fout,"MOV R1,\"Read\"\nPUSH R1\nMOV R1,-1\nPUSH R1\nPUSH R%d\nPUSH R1\nPUSH R1\nCALL 0\n",reg_no);
	fprintf(fout,"POP R0\nPOP R1\nPOP R1\nPOP R1\nPOP R1\nBRKP\n");
}

void XsmExit(){
    fprintf(fout,"MOV R1,\"Exit\"\nPUSH R1\nPUSH R1\nPUSH R1\nPUSH R1\nPUSH R1\nCALL 0\n");
	fprintf(fout,"POP R0\nPOP R1\nPOP R1\nPOP R1\nPOP R1\nBRKP\n");
}
