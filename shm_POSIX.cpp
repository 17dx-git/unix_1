#include <stdio.h>
#include <ctype.h>
#include <string.h> //для memset
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h> // ftruncate
// -lrt

const int MEM_SIZE = 1024*1024;
int main()
{
    const char * shm_name = "/test.shm";
    
    
    int shm_d = shm_open(shm_name, O_CREAT|O_RDWR, 0777 );         
    if ( shm_d == -1 ){
        perror("shm_open");
        return 1;
    }
    
    if ( ftruncate(shm_d, MEM_SIZE ) == -1){
        perror("ftruncate");
        return 1;
    }
    
    int prot = (PROT_READ | PROT_WRITE);
    int flags = MAP_SHARED;
    void *sh_mem = mmap(NULL, MEM_SIZE, prot, flags, shm_d, 0);
    if (sh_mem == (void *)-1){
        perror("mmap");
        return 1;
    }                  
                  
    memset (sh_mem, 13, MEM_SIZE);
       
    return 0;

}