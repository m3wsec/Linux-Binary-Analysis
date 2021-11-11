#include <sys/mman.h> //for mmap
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

void main()
{
    void* mem;
    int addr = 0x100000;
    int length = 1<<10;
    int fd = -1;
    int offset = 0;
    __asm__ __volatile__ (
        "push %%ebx                   \n\t"
        /*
        "push %%ecx                   \n\t"
        "push %%edx                   \n\t"
        
        "push %%esi                   \n\t"
        "push %%edi                   \n\t"
        */
        "push %%ebp                   \n\t"
        
        "mov $0xc0,%%eax              \n\t"
        "mov %0,%%ebx                \n\t"
        "mov %1,%%ecx                \n\t"
        "mov %2,%%edx                \n\t"
        "mov %3,%%esi                \n\t"
        "mov %4,%%edi                \n\t"
        "mov $0x0,%%ebp               \n\t"
        "int $0x80                  \n\t"
        
        "pop %%ebp                   \n\t"
        /*
        "pop %%edi                   \n\t"
        "pop %%esi                   \n\t"
        
        "pop %%edx                   \n\t"
        "pop %%ecx                   \n\t"
        */
        "pop %%ebx                   \n\t"
        
        :
        :"g"(addr),"g"(length),"g"(PROT_EXEC|PROT_READ|PROT_WRITE),"g"(MAP_ANONYMOUS|MAP_SHARED),"g"(fd)
    );

    __asm__ __volatile__(
        "mov %%eax,%0"
        :"=r"(mem)
    );
    printf("%p\n",mem);
    printf("%s\n",strerror(errno));
    
}