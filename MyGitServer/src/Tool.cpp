#include "Tool.h"
#include <cstdio>
#include <cstdlib>

void err_if(bool cond, const char* err_msg){
    if(cond){
        perror(err_msg);
        exit(EXIT_FAILURE);
    }
}