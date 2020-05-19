%{

/* Scan and return a token for identifiers of the format :
             string number
Note: strings are not case sensitive
Examples: a0 , A1 , ab2 , AB4 , aBc5
*/

#include<stdio.h>


int line = 0;
int words = 0;
int chars = 0;


%}

newline [\n]
space [ ]
character [a-zA-Z0-9]

%option noyywrap

%%

{newline} {
		line = line + 1;
		words = words + 1;}
		
{space} {
		words = words + 1;}
{character} {
		chars = chars + 1;}



%%

int main()
{
yylex();
printf("%d %d %d\n",chars,words,line);
  return 1;
}
