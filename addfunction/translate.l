%{
	#include <stdlib.h>
	#include <stdio.h>
	//#include "y.tab.h"
	//#include "lab_trans.h"
	int yyerror(char*);
    int line_add = 2047;
    int parse = 0;          // to know which iteration the parser is running(build lab_trans_table or lab_translation)
    int count = 0;              // to keep track of # of labels
    int line_count = 0;         // to keep track of first 8 header lines

    struct label_list{
        char* name;
        int add;
    }l_list[50];

    //GetAdd returns the address of the label sent as argument
    int GetAdd(struct label_list[] , char *);

    //GetLabel removes ':' from from the lexeme
    void GetLabel(char*);

    FILE *fin;
    FILE *fout;
%}

%%

L[0-9]+:[\n]    {
                l_list[count].name = strdup(yytext);
                GetLabel(l_list[count].name);
                l_list[count].add = line_add + 2;count = count + 1;}

F[0-9]+:[\n]    {
                l_list[count].name = strdup(yytext);
                GetLabel(l_list[count].name);
                l_list[count].add = line_add + 2;count = count + 1;
                }

MAIN:[\n]       {
                l_list[count].name = strdup(yytext);
                GetLabel(l_list[count].name);
                l_list[count].add = line_add + 2;count = count + 1;
                }

L[0-9]+[\n]     {if(parse != 0){
                    fprintf(fout,"%d\n",GetAdd(l_list,yytext));}
                else{
                fprintf(fout,"%s",yytext);line_add = line_add + 2;}
                }

F[0-9]+[\n]     {if(parse != 0){
                    fprintf(fout,"%d\n",GetAdd(l_list,yytext));}
                else{
                fprintf(fout,"%s",yytext);line_add = line_add + 2;}
                }

MAIN[\n]        {if(parse != 0){
                    fprintf(fout,"%d\n",GetAdd(l_list,yytext));}
                else{
                fprintf(fout,"%s",yytext);line_add = line_add + 2;}
                }


[\n]            {line_add = line_add + 2;fprintf(fout,"\n");line_count++;
                if(line_count == 8)
                    line_add--;
                }

^[0-9]+         {fprintf(fout,"%d",atoi(yytext));line_add = line_add - 1;}

.               {fprintf(fout,"%s",yytext);}       

%%

int yywrap(void) {
    parse++;
    fclose(fin);
    fclose(fout);
    fin = fopen("temp.xsm","r");
    yyin = fin;

    if(parse == 2)
	    return 1;
    fout = fopen("final.xsm","w");
    yylex();
    return 1;
}

void GetLabel(char* temp){
    int i = 1;
    while(*(temp + i) != ':')
        i++;
    *(temp + i) = '\n';
    *(temp + i + 1) = '\0';
}

int GetAdd(struct label_list list[], char* lab){
    int temp = 0;
    while(temp < count){
        if(strcmp(list[temp].name, lab) == 0){
            return list[temp].add;
        }
        temp++;
    }
}

int main(){
    
    fin = fopen("addfun.xsm","r");
    fout = fopen("temp.xsm","w");
    yyin = fin;
    yylex();
}