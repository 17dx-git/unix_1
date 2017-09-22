#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


void GetCommands(char *line, char **argv)
{
     char the_end = '\0';
     *argv++ = line; 
     while (*line != the_end) {       
          if (*line == '|'){
             *line='\0'; 
             *argv++ = ++line;            
          }
          ++line;
     }
     *argv = NULL; /* mark the end of argument list  */    
}

void  SplitString(char *line, char **argv)
{
     while (*line != '\0') {       /* if not the end of line ....... */ 
          while (*line == ' ' || *line == '\t' || *line == '\n')
               *line++ = '\0';     /* replace white spaces with 0    */
          *argv++ = line;          /* save the argument position     */
          if (*line == '\0') *(--argv)= NULL; 
          
          while (*line != '\0' && *line != ' ' && 
                 *line != '\t' && *line != '\n') 
               line++;             /* skip the argument until ...    */
     }
     *argv = NULL;                 /* mark the end of argument list  */
}

void ChangeFD(int old_fd, int new_fd){
    close(new_fd);
    if (dup2(old_fd, new_fd) == -1) { 
           std::cerr <<"ERROR: dup\n";
           exit(1);
    }
    close(old_fd);
}

void SettingsLastCommand( ){
       int fd = open("/home/box/result.out", O_RDWR | O_CREAT, 0777);
       if (fd ==-1) {    
           std::cerr <<"ERROR: file not open\n";
           exit(1);
       }      
       ChangeFD(fd, STDOUT_FILENO) ;
}


void ForkNext( char  ** commands) { 
   char the_end = '\0';
   if ( commands[1] == NULL )  {
       SettingsLastCommand( );
       char  *argv[64];
       SplitString( commands[0],argv );
       execvp( argv[0], &argv[0]);
   } ;
   
   int pid = fork();
   
   if (pid  < 0) {    
      std::cerr <<"ERROR: forking child process failed\n";
      exit(1);
   }
     
   int pfd[2];
   
   if (pipe(pfd) == -1) { 
       std::cerr <<"ERROR: not created pipe";
       exit(1); 
   }
   
   if (pid != 0){ //is parent
       ChangeFD(pfd[1], STDOUT_FILENO) ;
       close(pfd[0]); // ему не нужен 
       char  *argv[64];   
       SplitString( commands[0],argv );
       execvp( argv[0], &argv[0]);            
       pause(); 
   }
   else { //is child
       ChangeFD(pfd[0], STDIN_FILENO) ;
       close(pfd[1]); // ему не нужен       
       ForkNext(&commands[1]);
       char  *argv[64];
       SplitString( commands[1],argv );
       execvp( argv[0], &argv[0]);
       pause(); 
   }
}

/*void prnt(char **commands){
    for (char **i = commands; *i != NULL; i++){
       std::cout<<  *i << std::endl;
       char  *argv[64];   
       SplitString( commands[0],argv ); 
       for (char **j = argv; *j != NULL; j++){
            std::cout<<  *j << std::endl;
       } 
    }
    
    
}*/

int main(){
    std::string commands="who | sort -c | uniq  | sort ";
    //std::string commands="who ";
    std::cout<< commands;
// std::cin >> commands;
    char  *argv[64];
    GetCommands( (char *) commands.c_str(), argv );
    ForkNext( argv );
    prnt(argv);
    return 0;
}
