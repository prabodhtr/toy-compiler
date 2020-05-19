%{

#include <stdio.h>
#include <stdlib.h>
int yyerror();
%}

%union {
	char c;
};

%token DIGIT
%type <c> DIGIT

%left '+' '-'
%left '*'

%%

start : expr '\n'   {printf("\n");exit(1);}
	;   

expr:   expr '+' expr		{printf("+ ");}
	    | expr '*' expr		{printf("* ");}
		| expr '-' expr		{printf("- ");}
	    | '(' expr ')'
	    | DIGIT			    {printf("%d ",$<c>1);}	;

%%

yyerror()
{
	printf("Error");
}

main()
{
	yyparse();
	return 1;
}