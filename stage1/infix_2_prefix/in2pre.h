typedef struct tnode{
 int val; //value of the expression tree
 char *op; //indicates the opertor
 struct tnode *left,*right; //left and right branches
 }tnode;
	
/*Make a leaf tnode and set the value of val field*/
struct tnode* makeLeafNode(int n);
	
/*Make a tnode with opertor, left and right branches set*/
struct tnode* makeOperatorNode(char c,struct tnode *l,struct tnode *r);
	
/*To evaluate an expression tree*/
int evaluate(struct tnode *t);

//to get smallest free register
int GetReg();

//to release highest allocated register
int FreeReg();

//to generate header argument: no of variables to be read so as to adjust SP
void XsmHeader(int);

//to generate xsm code
int CodeGen(struct tnode*,FILE*);

//generated xsm code to call exit
void XsmExit();

//generate xsm code to call read
void XsmRead(int);

//generate xsm code to call write
void XsmWrite(int);

