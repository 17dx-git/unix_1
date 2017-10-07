#include <iostream>
#include <ctype.h>
#include <stdio.h> // fopen fprintf
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
// -lpthread

typedef void *(*start_routine) (void *);


bool writepid(){
   /////int fd = open("/home/dm/main.pid", O_RDWR | O_CREAT| O_TRUNC, 0777);
   FILE*  f = fopen( "/home/dm/main.pid", "w+" );
   if( !f ){    
           std::cerr <<"ERROR: file not open\n";
           return false;
   } 
   
   pid_t pid = getpid();
   fprintf( f, "%d", pid );
   //write(fd, &pid, sizeof(pid));
   fclose(f);  
   return true;
}

void* f_wait_mutex(void* mutex){
    pthread_mutex_lock( (pthread_mutex_t *) mutex);
    pthread_mutex_unlock(  (pthread_mutex_t *) mutex);    
    return mutex;
}

void* f_wait_spinlock(void* mutex){
    pthread_spin_lock( (pthread_spinlock_t *) mutex);
    pthread_spin_unlock(  (pthread_spinlock_t *) mutex);    
    return mutex;
}

void* f_wait_rlock(void* mutex){
    pthread_rwlock_rdlock( (pthread_rwlock_t *) mutex);
    pthread_rwlock_unlock(  (pthread_rwlock_t *) mutex);    
    return mutex;
}

void* f_wait_wlock(void* mutex){
    pthread_rwlock_wrlock( (pthread_rwlock_t *) mutex);
    pthread_rwlock_unlock(  (pthread_rwlock_t *) mutex);    
    return mutex;
}

bool spinlock_init(pthread_spinlock_t& spinlock){
   int pshared = PTHREAD_PROCESS_PRIVATE;
   int ret = pthread_spin_init(&spinlock, pshared);
   if (ret != 0){    
           std::cerr <<"ERROR: pthread_spin_init\n";
           return false;
   }  
    return true;
}

bool create_thread(pthread_t& thread_id,
                   void * arg,
                   start_routine f){
   int ret = pthread_create(&thread_id, NULL, f, arg);    
   if ( ret != 0 ){
        std::cerr << "create_wait_spinlock not was create";
        return false;
   }    
    return true;
}

const int THREADS_COUNT = 4;
int main()
{
   if (!writepid() ) return 1;
   
   pthread_t thread_id[THREADS_COUNT];
   
   pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
   pthread_mutex_lock( &mutex);   
   if ( !create_thread(thread_id[0], &mutex, f_wait_mutex ) ) return 1;
   
   pthread_spinlock_t spinlock;
   if ( !spinlock_init(spinlock) ) return 1;
   pthread_spin_lock( &spinlock);
   if ( !create_thread(thread_id[1], (void *) &spinlock, f_wait_spinlock) ) return 1;
   
   pthread_rwlock_t rlock = PTHREAD_RWLOCK_INITIALIZER;
   pthread_rwlock_rdlock(&rlock);
   if ( !create_thread(thread_id[2], &rlock, f_wait_rlock ) ) return 1;
   
   pthread_rwlock_t wlock = PTHREAD_RWLOCK_INITIALIZER;
   pthread_rwlock_wrlock( &wlock);
   if ( !create_thread(thread_id[3], &wlock, f_wait_wlock) ) return 1;
   
   sleep(4);
   
   pthread_mutex_unlock( &mutex);
   pthread_spin_unlock( &spinlock);
   pthread_rwlock_unlock( &rlock);
   pthread_rwlock_unlock( &wlock);
   
   for(int i = 0; i < THREADS_COUNT; i++){
        pthread_join(thread_id[i], NULL);
   }
   
   pthread_rwlock_destroy(&wlock);
   pthread_rwlock_destroy(&rlock);
   pthread_spin_destroy(&spinlock);
   pthread_mutex_destroy(&mutex);
       
   return 0;

}