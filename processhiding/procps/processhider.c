//gcc processhider.c -o processhider.so -fPIC -shared -ldl -g

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

struct dirent* readdir(DIR* dirp)
{
    struct dirent* (*target_readdir)(DIR*);

    target_readdir = dlsym((void*)0xFFFFFFFF,"readdir");
    
    
}

    
