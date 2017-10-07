#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>
//#include <cerrno>


const int MSG_MAXSIZE=1024;

int main(){
    const char * msgq_file = "/test.mq";
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10; /* максимальное число сообщений
                          в очереди в один момент времени */
    attr.mq_msgsize = MSG_MAXSIZE; /* максимальный размер очереди */
    attr.mq_curmsgs = 0;
    
    int mqd = mq_open( msgq_file, O_CREAT | O_RDWR, 0666, &attr );
    if( mqd == -1 ) {
         perror( "mq_open" );
        return 1;
    }
    

    int fd = open("/home/box/message.txt", O_RDWR | O_CREAT| O_TRUNC, 0777);
    if (fd ==-1) {    
           std::cerr <<"ERROR: file not open\n";
           exit(1);
    }  
    
    int msglen = 6;
    int msgprio=0;
    char *msgptr = (char*)malloc(msglen);
    sprintf(msgptr,"12345");
    if (  mq_send(mqd, msgptr, msglen, msgprio) ){
        std::cerr <<"ERROR: mq_send\n";
        return 1;
    };
       
    int n = 0;    
    char buff[attr.mq_msgsize+1];
    while ((n = mq_receive(mqd, buff, attr.mq_msgsize, NULL)) >= 0) {
       buff[n] = '\0';       
       std::cout << "i recive\n";
       write(fd, buff, n);
    }
    //std::cerr << errno<< std::endl;
    perror("recive");
    mq_close(mqd);
    mq_unlink(msgq_file);
    close(fd);
    
    return 0;
}
