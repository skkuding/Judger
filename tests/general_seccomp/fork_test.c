#include <unistd.h>
#include <stdio.h>
int main(){ pid_t p = fork(); if(p==0) return 0; printf("fork ret=%d\n", p); return 0; }
