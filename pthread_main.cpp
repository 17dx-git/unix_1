#include <iostream>
#include <ctype.h>
//#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
// -lpthread

void* thread_func(void* value){
    return value;
}
int main()
{
   int fd = open("/home/dm/main.pid", O_RDWR | O_CREAT| O_TRUNC, 0777);
       if (fd == -1) {    
           std::cerr <<"ERROR: file not open\n";
           return 1;
   } 
   
   pid_t pid = getpid();
   write(fd, &pid, sizeof(pid));
   close(fd);
   
   pthread_t thread_id;
   int ret = pthread_create(&thread_id, NULL, thread_func, NULL);
    
   if ( ret != 0 ){
        std::cerr << "thread not was create";
        return 1;
   }
   void *retval;
   ret = pthread_join(thread_id, &retval);
   if ( ret != 0 ){
        std::cerr << "pthread_join not success";
        return 1;
   }
    
   return 0;

}