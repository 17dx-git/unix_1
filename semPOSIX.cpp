#include <stdio.h>
#include <ctype.h>
//#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>
// -lpthread


int main()
{
    const char * sem_name = "/test.sem";
    
    unsigned int value = 66;
    sem_t * sem= sem_open(sem_name, O_CREAT, 0777, value ); 
    
    if ( sem == SEM_FAILED ){
        perror("sem_open");
        return 1;
    }
       
    return 0;

}