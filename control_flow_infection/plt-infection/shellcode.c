void _start()
{
    __asm__ __volatile__ (
        "mov $0x401030,%rax      \n\t"
        "jmp *%rax"
    );
}