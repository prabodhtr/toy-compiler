int Ar = -1,label = -1;
int label_Ar[20],lab_count = 0;      //to keep track of labels
int varAr[26] = {0};

struct tnode* createTree(int val, int nodetype, char* c, int optype, struct tnode *l,struct tnode *m, struct tnode *r){
    struct tnode* temp;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->val = val;
    temp->optype = optype;
    temp->varname = c;
    temp->nodetype = nodetype;
    temp->left = l;
    temp->mid = m;
    temp->right = r;
    printf("%d\n",temp->nodetype);
    //type checks

    switch(temp->nodetype){
        
        case PLUS_OP   :
        case MINUS_OP  :
        case MUL_OP    :
        case DIV_OP    :    if((temp->left)->optype != INTTYPE || (temp->right)->optype != INTTYPE ){
                                printf("type mismatch\n");
                                exit(0);
                            }
                            break;
    }
    return temp;
}

int evaluate(struct tnode* t){
    int p,q;
    switch(t->nodetype){
        case CONSTANT   :   return t->val;
                            break;

        case VAR        :   p = (int)(*(t->varname)) - 97;
                            return varAr[p];
                            break;

        case READ_NODE  :   //printf("read_node\n");
                            p = (int)(*((t->left)->varname)) - 97;
                            scanf("%d",&varAr[p]);
                            return 1;
                            break;

        case WRITE_NODE :   p = evaluate(t->left);
                            printf("%d\n",p);
                            return 1;
                            break;
                        
        case ASSIGN_NODE:   p = (int)(*((t->left)->varname)) - 97;
                            q = evaluate(t->right);
                            varAr[p] = q;
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

        case RELOP_LT         :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return (p < q);

        case RELOP_GT         :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return (p > q);

        case RELOP_LE         :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return (p <= q);

        case RELOP_GE         :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return (p >= q);

        case RELOP_EQ         :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return (p == q);

        case RELOP_NE         :   p = evaluate(t->left);
                            q = evaluate(t->right);
                            return (p != q);

        case IF_COND         :   p = evaluate(t->left);
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
                            
        case WHILE_LOOP      :  while(evaluate(t->left) != 0){
                                    p = evaluate(t->right);
                                    if(p == -2)
                                        break;
                                    else if(p == -1)    //not needed as any value other than -2 makes while loop runs again.continue affects CONNECT_NODE
                                        continue;
                                }
                                break;

        case DO_WHILE   :       do{
                                    p = evaluate(t->right);
                                    if(p == -2)
                                        break;
                                }while(evaluate(t->left) != 0);
                                break;

        case BREAK_NODE :   return -2;
                            break;

        case CONTINUE_NODE  :   return -1;
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

        default         :   printf("invalid statement\n");
    
    }
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

int CodeGen(struct tnode* t,FILE* fout){
    int p,q,r,s,label_1,label_2;
    switch (t->nodetype){
        case CONSTANT   :   p = GetReg();
                            if(p == -1)
                                return p;
                            fprintf(fout,"MOV R%d,%d\n",p,t->val);
                            return p;
                            break;

        case VAR        :   p = (int)(*(t->varname));
                            p = p - 97;
                            q = GetReg();
                            fprintf(fout,"MOV R%d,%d\n",q,4096+p);      //return reg_no of reg containing address of var
                            return q;
                            break;

        case READ_NODE:     p = CodeGen(t->left,fout);
                            if(p == 1){
                                q = GetReg();
                                fprintf(fout,"MOV R%d,R%d\n",q,p);
                                p = q;
                            }
                            FreeReg();
                            XsmRead(p);
                            return p;
                            break;

        case WRITE_NODE:    p = CodeGen(t->left,fout);
                            if((t->left)->nodetype == VAR){
                                q = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",q,p);
                                p = q;
                            }
                            XsmWrite(p);
                            FreeReg();
                            return p;
                            break;

        case ASSIGN_NODE:   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->right)->nodetype == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\nMOV [R%d],R%d\n",r,q,p,r);
                            }
                            else
                            {
                                fprintf(fout,"MOV [R%d],R%d\n",p,q);
                            }
                            FreeReg();
                            return p;
                            break;

        case PLUS_OP    :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"ADD R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;
                                                                
        case MINUS_OP   :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"SUB R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case MUL_OP     :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();                          
                            }
                            fprintf(fout,"MUL R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case DIV_OP     :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,q);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"DIV R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case RELOP_LT         :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"LT R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case RELOP_LE         :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"LE R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case RELOP_GT         :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"GT R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case RELOP_GE         :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"GE R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case RELOP_EQ         :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                fprintf(fout,"MOV R%d,R%d\n",q,s);
                                FreeReg();
                            }
                            fprintf(fout,"EQ R%d,R%d\n",p,q);
                            FreeReg();
                            return p;
                            break;

        case RELOP_NE         :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->nodetype == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                fprintf(fout,"MOV R%d,R%d\n",p,r);
                                FreeReg();
                            }
                            if((t->right)->nodetype == VAR){
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
    fprintf(fout,"0\n2056\n0\n0\n0\n0\n0\n0\nMOV SP,%d\n",4121);
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
