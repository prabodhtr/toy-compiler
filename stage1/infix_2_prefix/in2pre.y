%{
	#include <stdlib.h>
	#include <stdio.h>
	FILE* fout;
	#include "in2pre.h"
	#include "in2pre.c"
	int yylex(void);
	

%}

%union{
	struct tnode *no;
	
}
%type <no> expr NUM program END
%token NUM PLUS MINUS MUL DIV END
%left PLUS MINUS
%left MUL DIV

%%

program : expr END	{
				$$ = $2;
				printf("postorder :");
				postorder($1);
				printf("preorder :");
				preorder($1);
				CodeGen($1,fout);
				XsmWrite(0);
				printf("Answer : %d\n",evaluate($1));
				XsmExit();
				exit(1);
			}
		;

expr : expr PLUS expr		{$$ = makeOperatorNode('+',$1,$3);}
	 | expr MINUS expr  	{$$ = makeOperatorNode('-',$1,$3);}
	 | expr MUL expr	{$$ = makeOperatorNode('*',$1,$3);}
	 | expr DIV expr	{$$ = makeOperatorNode('/',$1,$3);}
	 | '(' expr ')'		{$$ = $2;}
	 | NUM			{$$ = $1;}
	 ;

%%

yyerror(char const *s)
{
    printf("yyerror %s",s);
}


int main(void) {

	fout = fopen("in2pre.xsm","w");
	XsmHeader(1);

	yyparse();

	return 0;
}
