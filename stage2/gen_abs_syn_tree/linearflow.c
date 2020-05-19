int Ar = -1;
int varAr[26] = {0};

struct tnode* createTree(int val, int type, char* c, struct tnode *l, struct tnode *r){
    struct tnode* temp;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->val = val;
    temp->type = type;
    temp->varname = c;
    temp->nodetype = type;
    temp->left = l;
    temp->right = r;
    //fprintf(fout,"%d\n",type);
    return temp;
}

int evaluate(struct tnode* t){
    int p,q;
    switch(t->type){
        case CONSTANT   :   return t->val;
                            break;

        case VAR        :   p = (int)(*(t->varname)) - 97;
                            return varAr[p];
                            break;

        case READ_NODE  :   //printf("read_node\n");
                            p = (int)(*((t->left)->varname)) - 97;
                            scanf("%d",&varAr[p]);
                            break;

        case WRITE_NODE :   p = evaluate(t->left);
                            printf("%d\n",p);
                            break;
                        
        case ASSIGN_NODE:   p = (int)(*((t->left)->varname)) - 97;
                            q = evaluate(t->right);
                            varAr[p] = q;
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

        case CONNECT_NODE:  evaluate(t->left);
                            evaluate(t->right);
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

int CodeGen(struct tnode* t,FILE* fout){
    int p,q,r,s;
    switch (t->type){
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
                            if((t->left)->type == VAR){
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
                            if((t->right)->type == VAR){
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
                            if((t->left)->type == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                p = r;
                            }
                            if((t->right)->type == VAR){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                q = s;
                            }
                            if(p < q){
                                fprintf(fout,"ADD R%d,R%d\n",p,q);
                                FreeReg();
                                return p; 
                            }
                            else{
                                fprintf(fout,"ADD R%d,R%d\n",q,p);
                                FreeReg();
                                return q;
                            }
                            break;
                                                                
        case MINUS_OP   :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->type == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                p = r;
                            }
                            if((t->right)->type == VAR){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                q = s;
                            }
                            if(p < q){
                                fprintf(fout,"SUB R%d,R%d\n",p,q);
                                FreeReg();
                                return p; 
                            }
                            else{
                                fprintf(fout,"SUB R%d,R%d\n",q,p);
                                FreeReg();
                                return q;
                            }
                            break;

        case MUL_OP     :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->type == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                p = r;
                            }
                            if((t->right)->type == VAR){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                q = s;
                            }
                            if(p < q){
                                fprintf(fout,"MUL R%d,R%d\n",p,q);
                                FreeReg();
                                return p; 
                            }
                            else{
                                fprintf(fout,"MUL R%d,R%d\n",q,p);
                                FreeReg();
                                return q;
                            }
                            break;

        case DIV_OP     :   p = CodeGen(t->left,fout);
                            q = CodeGen(t->right,fout);
                            if((t->left)->type == VAR){
                                r = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",r,p);
                                p = r;
                            }
                            if((t->right)->type == VAR){
                                s = GetReg();
                                fprintf(fout,"MOV R%d,[R%d]\n",s,q);
                                q = s;
                            }
                            if(p < q){
                                fprintf(fout,"DIV R%d,R%d\n",p,q);
                                FreeReg();
                                return p; 
                            }
                            else{
                                fprintf(fout,"DIV R%d,R%d\n",q,p);
                                FreeReg();
                                return q;
                            }
                            break;

        case CONNECT_NODE : CodeGen(t->left,fout);
                            CodeGen(t->right,fout);
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
