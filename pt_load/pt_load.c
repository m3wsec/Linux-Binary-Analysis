//Usage:./pt_load -f <filename> -o/-n (need nopie)
//change pt_note to pt_load / new pt_load
//64bit

#include <getopt.h>
#include <fcntl.h>
#include <stdio.h> //for fopen/fread/fwrite
#include <elf.h> // for elf file struct

#define FILE_SIZE_MAX 0x100000
#define PT_LOAD_ADDR 0x4FF000

const char* shortopts = "f:on";
struct option longopts[] = {
	{"file",required_argument,NULL,'f'},
	{"overlap",no_argument,NULL,'o'},
	{"new",no_argument,NULL,'n'}
};

unsigned char file_buf[FILE_SIZE_MAX];

void overlap(char* buf)
{
	//find PT_NOTE segment entry , overlap p_type to PT_LOAD and adjust other things
	Elf64_Ehdr* Elf64Header = (Elf64_Ehdr*)buf;
	
	unsigned int phoffset = Elf64Header->e_phoff;
	unsigned int phnum = Elf64Header->e_phnum;
	Elf64_Phdr* Elf64ProgramHeader = (Elf64_Phdr*)(buf+phoffset);
	
	for(int i=0; i<phnum ;i++)
	{
		if((Elf64ProgramHeader+i)->p_type == 0x4)
		{
			(Elf64ProgramHeader+i)->p_type = 0x1;
			(Elf64ProgramHeader+i)->p_vaddr = PT_LOAD_ADDR;
			(Elf64ProgramHeader+i)->p_paddr = PT_LOAD_ADDR;
			(Elf64ProgramHeader+i)->p_filesz = 0x1000;
			(Elf64ProgramHeader+i)->p_memsz = 0x1000;
			(Elf64ProgramHeader+i)->p_flags = (Elf64ProgramHeader+i)->p_flags | PF_X;
			break;
		}
	}
	
}

int main(int argc, char** argv)
{
    int c;
	FILE* fp;

    while( (c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1)
	{
		switch(c)
		{
			case 'f':
				fp = fopen(argv[2],"r+");
				fread(file_buf,sizeof(char),FILE_SIZE_MAX,fp);
				//fread change the position of file pointer , we neet set to 0 
    			fseek(fp,0,SEEK_SET);
				break;
			case 'o':
				overlap(file_buf);
				break;
				
			case 'n':
				//new(file_buf);
				break;
		}
	}
	
	fwrite(file_buf,sizeof(char),FILE_SIZE_MAX,fp);
	fclose(fp);
}