#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

const int SEMS_COUNT=16;

int main()
{
    key_t key = ftok("/tmp/sem.temp", 1);
    if (key == -1){
        perror("ftok");
        return 1;
    }
    int semflg = (IPC_CREAT | 0777);
    
    int semid = semget(key, SEMS_COUNT, semflg ); // sid
    struct sembuf sops[SEMS_COUNT];
    for (int i = 0; i < SEMS_COUNT; i++) {
        sops[i].sem_num = i;
        sops[i].sem_op = i;
        sops[i].sem_flg = 0;  //IPC_NOWAIT || SEM_UNDO
    }
    if ( semop(semid, sops, SEMS_COUNT) == -1 ){
        perror("semop");
        return 1;
    }
       
    return 0;

}