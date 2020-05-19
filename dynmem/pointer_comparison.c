#include<stdio.h>
int main(){
    int* ptr1,*ptr2;
    int a;
    a = 10;
    if(ptr1 == ptr2)
        printf("valid1\n");
    ptr1 = &a;
    ptr2 = &a;
    if(ptr1 == ptr2)
        printf("valid2\n");
}