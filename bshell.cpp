#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


char ** GetLastCommands(char *line, char **argv)
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
     *argv = '\0';                 /* mark the end of argument list  */    
     return --argv;
}

void  SplitString(char *line, char **argv)
{
     while (*line != '\0') {       /* if not the end of line ....... */ 
          while (*line == ' ' || *line == '\t' || *line == '\n')
               *line++ = '\0';     /* replace white spaces with 0    */
          *argv++ = line;          /* save the argument position     */
          if(*line == '\0') *(--argv) = '\0';
          while (*line != '\0' && *line != ' ' && 
                 *line != '\t' && *line != '\n') 
               line++;             /* skip the argument until ...    */
     }
     *argv = '\0';                 /* mark the end of argument list  */
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
       int fd = open("/home/dm/unix_1/result.out", O_RDWR | O_CREAT, 0777);
       if (fd ==-1) {    
           std::cerr <<"ERROR: file not open\n";
           exit(1);
       }      
       ChangeFD(fd, STDOUT_FILENO) ;
}


void ForkBack( char  ** commands) { 
   if ( commands[-1] == NULL )  {
       char  *argv[64];   
       SplitString( commands[0],argv );
       execvp( argv[0], &argv[0]);            
   } ;

  
   int pfd[2];
   
   if (pipe(pfd) == -1) { 
       std::cerr <<"ERROR: not created pipe";
       exit(1); 
   }
 
   int pid = fork();
   
   if (pid  < 0) {    
      std::cerr <<"ERROR: forking child process failed\n";
      exit(1);
   }
     
   
   if (pid != 0){ //is parent
       ChangeFD(pfd[0], STDIN_FILENO) ;
       close(pfd[1]); // ему не нужен       
       char  *argv[64];   
       SplitString( commands[0],argv );
       execvp( argv[0], &argv[0]);            
   }
   else { //is child
       ChangeFD(pfd[1], STDOUT_FILENO) ;
       close(pfd[0]); // ему не нужен 
       ForkBack(&commands[-1]);
   }
}


int main(){
    //std::string commands="who | sort -c | uniq  | sort \n";
    std::string commands="who |sort  ";
    //std::string commands;
    //std::cout<< commands;
    std::getline(std::cin , commands,'\n');
    char  *argv[64];
    argv[0]='\0';
    char** lastCommand = GetLastCommands( (char *) commands.c_str(), &argv[1] );
    SettingsLastCommand( );
    ForkBack( lastCommand );
    return 0;
}
