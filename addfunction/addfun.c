#include "addfun.h"
#include <stdio.h>
extern LocSymTable* LocSymTableHeader;

int Ar = -1,label = -1,sp = 4096,FunLabel = -1;
int label_Ar[20],lab_count = 0,var_count = 0;       // to keep track of labels
int cur_fun_loc_var_count = 0;                      // to keep track of local var to be popped out
tnode* cur_fun_node;                                // to know the function being executed right now
int GetType(struct tnode*,LocSymTable* LSTHeader);  // to get type of a node
void check(struct tnode*);                          // check array index and variable 
Par_List* AddToParamList(Par_List* plistheader,int type, char* name);
void FunSignatureCheck(char* funname,Par_List* paramlistheader,int ret_type);
LocSymTable* AddtoLocSymTable(LocSymTable* LocSymTableHeader, char* name, int type,int ArgFlag);
LocSymTable* LSTLookup(LocSymTable* LSTHeader, char *name);

gst_entry* gst_header = NULL;

struct var
{
    char* name;
    int val;
}varAr[50];

struct tnode* createTree(int val, int nodetype, char* c, int optype, struct tnode *l,struct tnode *m, struct tnode *r){
    struct tnode* temp;
    int type_l;
    int type_r;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->val = val;
    temp->optype = optype;
    temp->varname = c;
    temp->nodetype = nodetype;
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
                            if(type_r != type_l || type_r != INTTYPE){
                                printf("type mismatch\n");
                                exit(1);
                            }
                            break;

        case ASSIGN_NODE:   type_l = GetType(temp->left,LocSymTableHeader);
                            
                            type_r = GetType(temp->right,LocSymTableHeader);
                            if(type_r != type_l)
                                if(!(type_r == INTADDRTYPE && type_l == INTPTR))
                                    if(!(type_r == STRADDRTYPE && type_l == STRPTR)){
                                        printf("type mismatch\n");
                                        exit(1);
                                    }
                            break;

        case WHILE_LOOP:
        case DO_WHILE:
        case IF_COND:       type_l = GetType(temp->left,LocSymTableHeader);
                            if(type_l != BOOLTYPE){
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
                            if(type_l != type_r || (type_l == BOOLTYPE || type_r == BOOLTYPE)){
                                printf("type mismatch\n");
                                exit(1);
                            }
                            break;

        case WRITE_NODE :   type_l = GetType(temp->left,LocSymTableHeader);
                            if(type_l != INTTYPE && type_l != STRTYPE){
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

int GetType(struct tnode* t,LocSymTable* LocSymTableHeader){
    gst_entry* gst_ptr;
    LocSymTable* LST_ptr;
    switch(t->nodetype){
        case CONSTANT   :   return t->optype;
                            break;

        case POINTER_NODE:  LST_ptr = LSTLookup(LocSymTableHeader,t->varname);
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

        case PLUS_OP    :
        case MINUS_OP   :
        case MUL_OP     :
        case DIV_OP     :   
        case MODULO_OP  :   return INTTYPE;             
                            break;

        case RELOP_EQ   :
        case RELOP_GE   :
        case RELOP_GT   :
        case RELOP_LE   :
        case RELOP_LT   :
        case RELOP_NE   :   return BOOLTYPE;
                            break;

        case ADDRESS_NODE:  LST_ptr = LSTLookup(LocSymTableHeader,t->varname);
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
    int type_r,type_l;
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
        if(type_l != INTTYPE){
            printf("invalid array index\n");
            exit(1);
        }
        if(t->right != NULL){
            type_r = GetType(t->right,LocSymTableHeader);
            if(type_r != INTTYPE){
                printf("invalid array index\n");
                exit(1);
            }
        }
    }
}

int GetVarIndex(char* name){
    int count = 0;
    while(count < var_count){
        if(strcmp(varAr[count].name, name) == 0)
            return count;
        count = count +1 ;
        }
}

Par_List* AddToParamList(Par_List* parlistheader,int type, char* name){
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

LocSymTable* AddtoLocSymTable(LocSymTable* LocSymTableHeader, char* name, int type,int ArgFlag){
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
    new->binding = temp->binding + -1;
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

void FunSignatureCheck(char* funname,Par_List* paramlistheader,int ret_type_chk_flag){
    gst_entry* gst_ptr;
    gst_ptr = Lookup(funname);
    if(gst_ptr == NULL){
        printf("function undeclared\n");
        exit(1);
    }
    if(ret_type_chk_flag != -1){
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
            printf("%d-%d\n",paramlistheader->type,fun_parlist_header->type);
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

int Install(char *name, int type, int size,int col_size,Par_List* paramlistheader){
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
    Ar++;
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

int CodeGen(struct tnode* t,FILE* fout){
    int p,q,r,s,label_1,label_2;
    int i,j;
    gst_entry* gst_ptr;
    tnode* ASTnode;
    Par_List* ArgList;
    LocSymTable* LSTentry,*LSTHeader;
    switch (t->nodetype){
        case CONSTANT   :   p = GetReg();
                            if(p == -1)
                                return p;
                            if(t->optype == INTTYPE)
                                fprintf(fout,"MOV R%d,%d\n",p,t->val);
                            else
                                fprintf(fout,"MOV R%d,%s\n",p,t->varname);
                            return p;
                            break;

        case VAR        :   LSTentry = LSTLookup(cur_fun_node->lst_ptr,t->varname);
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
                                    fprintf(fout,"MOV R%d,BP\nMOV R%d,%d\n",p,q,LSTentry->binding);
                                    fprintf(fout,"ADD R%d,R%d\n",p,q);
                                    FreeReg();
                                }
                            }
                            return p;            //return the address of the location to which var is statically bounded
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
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE){
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

        case ASSIGN_NODE:   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\nMOV R%d,R%d\n",r,q,q,r);
                                FreeReg();
                            }
                            fprintf(fout,"MOV [R%d],R%d\n",p,q);
                            FreeReg();
                            FreeReg();
                            return p;
                            break;

        case PLUS_OP    :   //Match_int_optype(t);
                            p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE){
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
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE){
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
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE){
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
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE){
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
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE){
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
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE){
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
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE){
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
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE){
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
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE){
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
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE){
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
                            if((t->left)->nodetype == VAR || (t->left)->nodetype == POINTER_NODE){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR || (t->right)->nodetype == POINTER_NODE){
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
                                if(ASTnode->nodetype == VAR){
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

        default         :   printf("invalid nodetype\n");
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
