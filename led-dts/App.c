#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/types.h"
#include "fcntl.h"
#include "sys/stat.h"

int main(int argc,char **argv)
{
    int fd,ret,flag;
    if(argc < 3)
    {
        printf("cmd error:eg below->\r\n \
        leddrv /dev/xxx 0|1\r\n");
        return -1;
    }        
    fd = open(argv[1],O_RDWR);
    if(fd < 0)
    {
        printf("dev open failed!\r\n");
        return -1;
    }
    flag = atoi(argv[2]);
    ret = write(fd,(unsigned char*)&(flag),1);
    if(ret < 0)
    {
        printf("write failed!\r\n");
        return -1;
    }
    close(fd);
    return 0;
}