int Ar[20] = {0};

struct tnode* makeLeafNode(int n)
{
    struct tnode *temp;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->op = NULL;
    temp->val = n;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

struct tnode* makeOperatorNode(char c,struct tnode *l,struct tnode *r){
    struct tnode *temp;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->op = malloc(sizeof(char));
    *(temp->op) = c;
    temp->left = l;
    temp->right = r;
    return temp;
}

int evaluate(struct tnode *t){
    if(t->op == NULL)
    {
        return t->val;
    }
    else{
        switch(*(t->op)){
            case '+' : return evaluate(t->left) + evaluate(t->right);
                       break;
            case '-' : return evaluate(t->left) - evaluate(t->right);
                       break;
            case '*' : return evaluate(t->left) * evaluate(t->right);
                       break;
            case '/' : return evaluate(t->left) / evaluate(t->right);
                       break;
        }
    }
}
void preorder(struct tnode *t)
{
    
    if(t->op != NULL){
        printf("%c ",*(t->op));
    }
    else{
        printf("%d ",t->val);
        return;
    }
    preorder(t->left);
    preorder(t->right);

}

void postorder(struct tnode *t)
{
    if(t->left == NULL){
        printf("%d ",t->val);
        return;
    }
    postorder(t->left);
    postorder(t->right);
    printf("%c ",*(t->op));
}

int GetReg(){
    int i = 0;
    for(;i < 20;i++){
        if(Ar[i] == 0){
            Ar[i] = 1;
            return i;
        }
    if(i == 20)
        return -1;
    }
}

int FreeReg(){
    int i = 19;
    for(;i >= 0;i--){
        if(Ar[i] == 1){
            Ar[i] = 0;
            return i;
        }
    if(i == 0)
        return -1;
    }    
}

int CodeGen(struct tnode *t,FILE* fout){

    int p,q;
    // store value of leaf nodes
    if( t->left == NULL ){
        p = GetReg();
        if(p == -1)
            return -1;
        fprintf(fout,"MOV R%d,%d\n",p, t->val);
        return p;
    }
    else
    {
        p = CodeGen(t->left,fout);
        q = CodeGen(t->right,fout);
        switch (*(t->op)){
            case '+'    :   fprintf(fout,"ADD R%d,R%d\nBRKP\n",p,q);
                            
                            break;

            case '-'    :   fprintf(fout,"SUB R%d,R%d\n",p,q);
                            break;

            case '*'    :   fprintf(fout,"MUL R%d,R%d\n",p,q);
                            break;

            case '/'    :   fprintf(fout,"DIV R%d,R%d\n",p,q);
                            break;                            
        }

        FreeReg();
        return p;
    }

}

void XsmHeader(int var2read){
    fprintf(fout,"0\n2056\n0\n0\n0\n0\n0\n0\nMOV SP,%d\n",4095+var2read);
}

void XsmWrite(int r_num){
    fprintf(fout,"MOV R1,\"Write\"\nPUSH R1\nMOV R1,-2\nPUSH R1\nPUSH R%d\nPUSH R1\nPUSH R1\nCALL 0\n",r_num);
	fprintf(fout,"POP R0\nPOP R1\nPOP R1\nPOP R1\nPOP R1\nBRKP\n");
}

void XsmRead(int v_num){
    fprintf(fout,"MOV R1,\"Read\"\nPUSH R1\nMOV R1,-1\nPUSH R1\nPUSH %d\nPUSH R1\nPUSH R1\nCALL 0\n",4095+v_num);
	fprintf(fout,"POP R0\nPOP R1\nPOP R1\nPOP R1\nPOP R1\nBRKP\n");
}

void XsmExit(){
    fprintf(fout,"MOV R1,\"Exit\"\nPUSH R1\nPUSH R1\nPUSH R1\nPUSH R1\nPUSH R1\nCALL 0\n");
	fprintf(fout,"POP R0\nPOP R1\nPOP R1\nPOP R1\nPOP R1\nBRKP\n");
}