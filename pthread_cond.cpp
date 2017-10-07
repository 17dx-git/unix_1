#include <iostream>
#include <ctype.h>
#include <stdio.h> // fopen fprintf
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
// -lpthread

typedef void *(*start_routine) (void *);

struct SDataMutex{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} ;


bool writepid(){
   /////int fd = open("/home/dm/main.pid", O_RDWR | O_CREAT| O_TRUNC, 0777);
   FILE*  f = fopen( "/home/dm/main.pid", "w+" );
   if( !f ){    
           std::cerr <<"ERROR: file not open\n";
           return false;
   } 
   
   pid_t pid = getpid();
   fprintf( f, "%d", pid );
   fclose(f);  
   return true;
}

void* f_wait_cond(void* arg){
    SDataMutex* data = (SDataMutex *) arg;
    pthread_cond_wait( &data->cond, &data->mutex );  
    return arg;
}

void* f_wait_barrier(void* arg){
    pthread_barrier_wait( (pthread_barrier_t*) arg );  
    return arg;
}


bool barrier_init(pthread_barrier_t& barrier){
   unsigned count =1;
   int ret = pthread_barrier_init(&barrier, NULL, count);
   if (ret != 0){    
           std::cerr <<"ERROR: pthread_barrier_init\n";
           return false;
   }  
    return true;
}


bool create_thread(pthread_t& thread_id,
                   void * arg,
                   start_routine f){
   int ret = pthread_create(&thread_id, NULL, f, arg);    
   if ( ret != 0 ){
        std::cerr << "pthread_create not was create";
        return false;
   }    
    return true;
}

const int THREADS_COUNT = 2;
int main()
{
   if (!writepid() ) return 1;
   
   pthread_t thread_id[THREADS_COUNT];
   SDataMutex data;
   data.cond = PTHREAD_COND_INITIALIZER;
   data.mutex = PTHREAD_MUTEX_INITIALIZER;   
   if ( !create_thread(thread_id[0], &data, f_wait_cond) ) return 1;
   

   pthread_barrier_t  barrier;
   if ( !barrier_init(barrier) ) return 1;
   if ( !create_thread(thread_id[1], &barrier, f_wait_barrier ) ) return 1;
   
   sleep(4);
   
   pthread_cond_signal( &data.cond );

   for(int i = 0; i < THREADS_COUNT; i++){
        pthread_join(thread_id[i], NULL);
   }
   
   pthread_barrier_destroy(&barrier);
   pthread_mutex_destroy(&data.mutex);
   pthread_cond_destroy(&data.cond);   
       
   return 0;

}