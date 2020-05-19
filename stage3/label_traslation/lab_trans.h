#define CONSTANT 0
#define VAR 1
#define READ_NODE 2
#define WRITE_NODE 3
#define ASSIGN_NODE 4
#define PLUS_OP 5
#define MINUS_OP 6
#define MUL_OP 7
#define DIV_OP 8
#define RELOP_LT 9
#define RELOP_LE 10
#define RELOP_GT 11
#define RELOP_GE 12
#define RELOP_EQ 13
#define RELOP_NE 14
#define IF_COND 15 
#define WHILE_LOOP 16
#define BREAK_NODE 17
#define CONTINUE_NODE 18
#define DO_WHILE 19
#define CONNECT_NODE -1
#define INTTYPE 1
#define BOOLTYPE 0

FILE* fin;
FILE* fout;


typedef struct tnode { 
	int val;	// value of a number for NUM nodes.
	int optype;	//type of variable
	char* varname;	//name of a variable for ID nodes  
	int nodetype;  // information about non-leaf nodes - read/write/connector/+/* etc.  
	struct tnode *left,*mid,*right;	//left and right branches   
}tnode;

/*Create a node tnode*/
struct tnode* createTree(int val, int nodetype, char* c, int optype, struct tnode *l,struct tnode* mid, struct tnode *r);
	
/*To evaluate an expression tree*/
int evaluate(struct tnode *t);

//to get smallest free register
int GetReg();

//to release highest allocated register
int FreeReg();

//to generate header argument: no of variables to be read so as to adjust SP
void XsmHeader();

//to generate xsm code
int CodeGen(struct tnode*,FILE*);

//generated xsm code to call exit
void XsmExit();

//generate xsm code to call read
void XsmRead(int);

//generate xsm code to call write
void XsmWrite(int);

