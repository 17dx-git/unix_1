#include <stdio.h>
#include <string.h> //для memset
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


const int MEM_SIZE = 1024;
int main()
{
    key_t key = ftok("/tmp/mem.temp", 1);
    if (key == -1){
        perror("ftok");
        return 1;
    }
    
    int shmflg = (IPC_CREAT | 0777);  
    int shmid = shmget(key, MEM_SIZE, shmflg );     
    if (shmid == -1){
        perror("shmget");
        return 1;
    }
    
    shmflg = (SHM_RND);
    void * sh_mem = shmat(shmid, NULL, shmflg );     
    if (sh_mem == (void *)-1){
        perror("shmat");
        return 1;
    }
    memset (sh_mem, 42, MEM_SIZE);
       
    return 0;

}