/*
    char* argv[] = {"/bin/bash",NULL};
    execve("/bin/bash",argv,NULL);
*/

void _start()
{
    char* path = "/bin/bash";
    //char* argv[] = {"/bin/bash",0};

    __asm__(
        "mov $0xb,%%eax \n\t"
        "mov %0,%%ebx   \n\t"
        "mov $0x0,%%ecx   \n\t"
        "mov $0x0,%%edx   \n\t"
        "int $0x80"
        :
        :"b"(path)
    );
}