void _start()
{
    __asm__ __volatile__ (
        "push $0x4011d5    \n\t"
        "ret               \n\t"
        "nop"
    );
}