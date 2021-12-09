void _start()
{
    //default relative address jmp , prefix "*" to absolute address jmp
    //absolute address jmp operand can not be immediate
    __asm__ __volatile__
    (
        "mov $0x8049080,%eax    \n\t"
        "jmp *%eax              \n\t"
    );
}
