%{
    #include<stdio.h>
    #include "y.tab.h"
%}

%%

[a-z]+ {
            yylval = *yytext;
            return DIGIT;}

"+"     {
            return *yytext;}

"-"     {
            return *yytext;}

"*"     {
            return *yytext;}

[()]	    return *yytext;

[\n]        return *yytext;


%%

yywrap()
{
	return 1;
}