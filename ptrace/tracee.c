#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void main()
{
    /*
    char buf[0x100];
    scanf("%s",buf);
    printf("%s\n",buf);
    */
    for(int i=0;i<200;i++)
    {
        sleep(1);
        printf("%d\n",i);
    }
}