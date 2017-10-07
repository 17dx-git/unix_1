#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/msg.h>


int main(){
    key_t key = ftok("/tmp/msg.temp", 1);
    if (key == -1){
        perror("ftok");
        return 1;
    }
    int  msgflg = (IPC_CREAT | 0777);
    int msqid = msgget (key, msgflg);
    if (msqid == -1){
        perror("msgget");
        return 1;
    }
    
    int fd = open("/home/box/message.txt", O_RDWR | O_CREAT| O_TRUNC, 0777);
    if (fd ==-1) {    
           std::cerr <<"ERROR: file not open\n";
           return 1;
    }  
    
    int n = 0; 
    long msgtyp = 0; //get any
    msgflg =0;   
    struct message { long mtype; char mtext[80]; } msg;
    while ((n = msgrcv(msqid, &msg, sizeof(msg), msgtyp, msgflg ) ) >= 0) {      
       std::cout << "i recive\n";
       write(fd, msg.mtext, strlen(msg.mtext)-1);
    }
    perror("recive");
    close(fd);
    
    return 0;
}
