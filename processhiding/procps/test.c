//gcc test.c -o test -g

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

void main()
{
    DIR* odir = opendir("/proc");
    struct dirent* dir = readdir(odir);
    printf("%s\n",dir->d_name);

    dir = readdir(odir);
    printf("%s\n",dir->d_name);
}