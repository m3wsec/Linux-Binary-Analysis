#include <sys/ptrace.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <fcntl.h>
#include <sys/stat.h> //for file length

#define MAX_BUF_SIZE 0x400

struct user_regs_struct backup_regs;
struct user_regs_struct current_regs;
unsigned int save_buf[MAX_BUF_SIZE >> 2] = {0};


int main(int argc, char** argv)
{
    int status;    

    int target = atoi(argv[1]);
    printf("Target PID:%d ptracing...\n",target);
    ptrace(PTRACE_ATTACH,target,NULL,NULL);
    pid_t cpid = wait(&status);
    printf("Target PID:%d STOPPED:%d . Status Code:%d\n",cpid,WIFSTOPPED(status),WSTOPSIG(status));
    
//save original regs
    ptrace(PTRACE_GETREGS,target,NULL,&backup_regs);
    printf("Current eip: %lx\n",backup_regs.eip);
    
//save original text data
    printf("Save Loop Count:%d\n",MAX_BUF_SIZE>>2);
    for(int i=0;i < (MAX_BUF_SIZE >> 2);i++)
    {
        save_buf[i] = ptrace(PTRACE_PEEKTEXT,target,backup_regs.eip+4*i,NULL);
    }
    
//inject mmap shellcode , end of int3 , return mmap addr
    //read shellcode file
    struct stat file_stat;
    stat("/tmp/mmap_shellcode",&file_stat);
    int file_size = file_stat.st_size;
    int fd = open("/tmp/mmap_shellcode",O_RDONLY);
    int mmap_shellcode[MAX_BUF_SIZE >> 2] = {0};
    read(fd,mmap_shellcode,file_size);
    //inject
    printf("File Size:%d\n",(file_size));
    printf("Inject Loop Count:%d\n",((file_size>>2)+1));
    for(int i=0;i<((file_size>>2)+1);i++)
    {
        printf("Origin Code:%lx\n",ptrace(PTRACE_PEEKTEXT,target,backup_regs.eip+4*i,NULL));
        ptrace(PTRACE_POKETEXT,target,backup_regs.eip+4*i,*(mmap_shellcode+i));
        printf("Current Code:%lx\n\n",ptrace(PTRACE_PEEKTEXT,target,backup_regs.eip+4*i,NULL));
    }
    //continue
    ptrace(PTRACE_CONT,target,NULL,NULL);
    //int3 receiver
    cpid = wait(&status);
    printf("Target PID:%d STOPPED:%d . Status Code:%d\n",cpid,WIFSTOPPED(status),WSTOPSIG(status));
    //retrive mmap return value
    ptrace(PTRACE_GETREGS,target,NULL,&current_regs);
    printf("Mmap Return Value at: %lx\n",current_regs.eax);
    //sleep(100);
//inject write shellcode in mmap area ,end of jmp original eip
    /*
    current_regs.eip = current_regs.eax;
    ptrace(PTRACE_SETREGS,target,NULL,&current_regs);
    //write shellcode
    */
//change eip ,restore eip and text data
    ptrace(PTRACE_GETREGS,target,NULL,&current_regs);
    printf("Current eip:%lx\n",current_regs.eip);
    ptrace(PTRACE_SETREGS,target,NULL,&backup_regs);
    ptrace(PTRACE_GETREGS,target,NULL,&current_regs);
    printf("After Restore eip:%lx\n",current_regs.eip);

    printf("Restore Loop Count:%d\n",MAX_BUF_SIZE>>2);
    for(int i=0;i < (MAX_BUF_SIZE >> 2);i++)
    {
        ptrace(PTRACE_POKETEXT,target,backup_regs.eip+(4*i),*(save_buf+i));
    }

    ptrace(PTRACE_CONT,target,NULL,NULL);
}