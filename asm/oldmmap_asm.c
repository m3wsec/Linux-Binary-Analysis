#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

struct mmap_arg_struct
{
    unsigned long addr;
	unsigned long len;
	unsigned long prot;
	unsigned long flags;
	unsigned long fd;
	unsigned long offset;
};

void main()
{
    struct mmap_arg_struct old_mmap_arg;
    old_mmap_arg.addr = 0x100000;
    old_mmap_arg.len = 0x1000;
    old_mmap_arg.prot = 0x7;
    old_mmap_arg.flags = 0x21;
    old_mmap_arg.fd = 0xffffffff;
    old_mmap_arg.offset = 0x0;

    __asm__ __volatile__ (
        "push %%ebx         \n\t"
        "mov $0x5a,%%eax    \n\r"
        "mov %0,%%ebx       \n\t"
        "int $0x80          \n\t"
        "pop %%ebx"
        :
        :"c"(&old_mmap_arg)
        ://"ebx"
    );
    printf("%s\n",strerror(errno));
}