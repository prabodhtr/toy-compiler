#include "sym_table.h"
#include <stdio.h>

int Ar = -1,label = -1,sp = 4096;
int label_Ar[20],lab_count = 0,var_count = 0;       //to keep track of labels
void Match_int_optype(struct tnode*);               // to type check arithmetic operations
int GetType(struct tnode*);                         // to get type of a node
void check(struct tnode*);                          //check array index and variable 

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
        case DIV_OP     :   type_l = GetType(temp->left);
                            type_r = GetType(temp->right);
                            if(type_r != type_l || type_r != INTTYPE){
                                printf("type mismatch\n");
                                exit(1);
                            }
                            break;

        case ASSIGN_NODE:   type_l = GetType(temp->left);
                            type_r = GetType(temp->right);
                            if(type_r != type_l)
                                if(!(type_r == INTADDRTYPE && type_l == INTPTR))
                                    if(!(type_r == STRADDRTYPE && type_l == STRPTR)){
                                        printf("type mismatch\n");
                                        exit(1);
                                    }
                            break;

        case WHILE_LOOP:
        case DO_WHILE:
        case IF_COND:       type_l = GetType(temp->left);
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
        case RELOP_NE   :   type_l = GetType(temp->left);
                            type_r = GetType(temp->right);
                            if(type_l != type_r || (type_l == BOOLTYPE || type_r == BOOLTYPE)){
                                printf("type mismatch\n");
                                exit(1);
                            }
                            break;

    }
    return temp;
}

int GetType(struct tnode* t){
    gst_entry* gst_ptr;
    switch(t->nodetype){
        case CONSTANT   :   return t->optype;
                            break;

        case POINTER_NODE:  gst_ptr = Lookup(t->varname);
                            if(gst_ptr->type == INTPTR)
                                return INTTYPE;
                            else 
                                return STRTYPE;
                            break;

        case VAR        :   gst_ptr = Lookup(t->varname);
                            return gst_ptr->type;
                            break;

        case PLUS_OP    :
        case MINUS_OP   :
        case MUL_OP     :
        case DIV_OP     :   
        case MODULO_OP  : return INTTYPE;             
                            break;

        case RELOP_EQ   :
        case RELOP_GE   :
        case RELOP_GT   :
        case RELOP_LE   :
        case RELOP_LT   :
        case RELOP_NE   :   return BOOLTYPE;
                            break;

        case ADDRESS_NODE:  gst_ptr = Lookup(t->varname);
                            if(gst_ptr->type == INTTYPE)
                                return INTADDRTYPE;
                            else
                                return STRADDRTYPE;
                            break;
    }
}

void check(struct tnode* t){
    gst_entry* gst_ptr;
    int type_r,type_l;
    gst_ptr = Lookup(t->varname);
    if(gst_ptr == NULL){
        printf("undeclared variable \"%s\"\n",t->varname);
        exit(1);
    }
    if(t->left != NULL){
        type_l = GetType(t->left);
        //printf("type %d\n",type_l);
        if(type_l != INTTYPE){
            printf("invalid array index\n");
            exit(1);
        }
        if(t->right != NULL){
            type_r = GetType(t->right);
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


struct Gsymbol* Lookup(char * name){
    gst_entry* temp = gst_header;
    while(temp != NULL){
        if(strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

int Install(char *name, int type, int size,int col_size){
    gst_entry* temp = gst_header;
    if(gst_header != NULL){
        while(temp->next != NULL){
            if(strcmp(temp->name, name) == 0){
                printf("Multiple declaration of variable %s\n",temp->name);  
                return -1;                      //variable already exists
            }
            temp = temp->next;
        }
    }
    gst_entry* new = (gst_entry*)malloc(sizeof(gst_entry));
    new->name = name;
    new->type = type;
    new->size = size;
    new->col_size = col_size;
    new->next = NULL;
    if(gst_header == NULL){
        new->binding = 4096;
        gst_header = new;
    }
    else{
        new->binding = temp->binding + (temp->size * temp->col_size);      //new location = start of prev loc + size of prev location
        temp->next = new;
    }
    printf("%s = %d\n",new->name,new->binding);

    return 1;
}


/*int evaluate(struct tnode* t){
    int p,q;
    switch(t->nodetype){
        case CONSTANT   :   return t->val;
                            break;

        case VAR        :   p = GetVarIndex(t->varname);
                            return varAr[p].val;
                            break;

        case READ_NODE  :   //printf("read_node\n");
                            p = GetVarIndex((t->left)->varname);
                            scanf("%d",&varAr[p].val);
                            return 1;
                            break;

        case WRITE_NODE :   p = evaluate(t->left);
                            printf("%d\n",p);
                            return 1;
                            break;
                        
        case ASSIGN_NODE:   p = GetVarIndex((t->left)->varname);
                            q = evaluate(t->right);
                            varAr[p].val = q;
                            return 1;
                            break;

        case PLUS_OP    :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return p + q;
                            break;

        case MINUS_OP   :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return p - q;
                            break;

        case MUL_OP     :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return p * q;
                            break;
        
        case DIV_OP     :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return p / q;
                            break;

        case RELOP_LT   :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return (p < q);

        case RELOP_GT   :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return (p > q);

        case RELOP_LE   :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return (p <= q);

        case RELOP_GE   :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return (p >= q);

        case RELOP_EQ   :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return (p == q);

        case RELOP_NE   :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return (p != q);

        case IF_COND    :   p = evaluate(t->left);
                            if(p == 1){
                                p = evaluate(t->mid);
                                return p;
                            }
                            else
                            {
                                if((t->right) != NULL){
                                     p = evaluate(t->right);
                                     return p;
                                }
                                return 0;
                            }
                            
        case WHILE_LOOP :   while(evaluate(t->left) == 1){
                                p = evaluate(t->right);
                                if(p == -2)
                                    break;
                                else if(p == -1)    //not needed as any value other than -2 makes while loop runs again.continue affects CONNECT_NODE
                                    continue;
                                }
                            break;

        case DO_WHILE   :   do{
                                p = evaluate(t->right);
                                if(p == -2)
                                    break;
                            }while(evaluate(t->left) != 0);
                            break;

        case BREAK_NODE :   return -2;
                            break;

        case CONTINUE_NODE: return -1;
                            break;

        case CONNECT_NODE:  p = evaluate(t->left);
                            if(p == -1)
                                return -1;
                            q = evaluate(t->right);
                            if(p == -2||q == -2)
                                return -2;
                            else if(q == -1)
                                return -1;
                            else 
                                return 1;
                            break;

       /* case DECL_NODE  :   t = t->right;
                            while(t != NULL){
                                if(t->nodetype != CONNECT_NODE){
                                    varAr[var_count].name = t->varname;
                                }
                                else
                                {
                                    varAr[var_count].name = (t->left)->varname;
                                }
                                t = t->right;
                                var_count++;
                            }
                            return 1;
                            break;

        default         :   printf("invalid statement\n");
    
    }
}*/

/*void Match_int_optype(struct tnode* t){
    gst_entry* gst_ptr;
    if((t->left)->nodetype == VAR){
        gst_ptr = Lookup((t->left)->varname);
        if(gst_ptr->type != INTTYPE){
            printf("type mismatch\n");
            exit(1);
        }
    }
    if((t->right)->nodetype == VAR){
        gst_ptr = Lookup((t->right)->varname);
        if(gst_ptr->type != INTTYPE){
            printf("type mismatch\n");
            exit(1);
        }
    }
}*/

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

int CodeGen(struct tnode* t,FILE* fout){
    int p,q,r,s,label_1,label_2;
    gst_entry* gst_ptr;
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

        case VAR        :   gst_ptr = Lookup(t->varname);
                            //printf("%s = %d\n",gst_ptr->name,gst_ptr->binding);
                            p = GetReg();

                            if(gst_ptr == NULL){
                                printf("undeclared variable \"%s\"\n",t->varname);
                                exit(1);
                            }

                            
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
                                fprintf(fout,"MOV R%d,%d\n",p,gst_ptr->binding);
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
    fprintf(fout,"0\n2056\n0\n0\n0\n0\n0\n0\nMOV SP,%d\n",sp);
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
