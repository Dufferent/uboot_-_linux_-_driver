#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/types.h"
#include "fcntl.h"
#include "sys/stat.h"

int main(int argc,char **argv)
{
    if(argc<4)
    {
        printf("less cmd!\r\ntry->[App <dev> <op> <str>]\r\n");
        exit(-1);
    }
    if(!strcmp(argv[2],"read"))
    {
        char buf[32]={0};
        int ret;
        int fd = open(argv[1],O_RDWR);
        if(fd < 0)
        {
            printf("dev open failed！\r\n");
            exit(-1);            
        }
        ret = read(fd,buf,32);
        if(ret < 0)
        {
            printf("read failed！\r\n");
            exit(-1);            
        }    
        printf("kernel data Rs:[%s]\r\n",buf);
        close(fd);
    }
    else if(!strcmp(argv[2],"write"))
    {
        char buf[32]={0};
        int ret;
        int fd = open(argv[1],O_RDWR);
        if(fd < 0)
        {
            printf("dev open failed！\r\n");
            exit(-1);            
        }
        ret = write(fd,argv[3],strlen(argv[3]));
        if(ret < 0)
        {
            printf("write failed！\r\n");
            exit(-1);            
        }
        close(fd);    
    }        
    return 0;
}