#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>

int max_tasks = 0;
int tasks=0;
const int READ = 0;
const int WRITE = 1;
const int ARG_MAX=1024;
const char whitespaces[]="  ";

int convert(char* number){
  int i=0;
  while(number[i]!='\0'){
    if(number[i]<'0' || number[i]>'9')
      errx(-1,"Invalid parameter");
    i++;
  }
  return atoi(number);
}

void split(char* line, char** args,const char* delim){
  char **next = args;
  char* tmp = strtok(line,delim);
  while (tmp != NULL){
    *next ++ = tmp;
    tmp = strtok(NULL,delim);
  }
  *next=NULL;
}

char* read_line(){
  ssize_t size=sizeof(char)*ARG_MAX;
  char* buff=(char*)malloc(size);
  if (buff == NULL )
      err(-1,"Memory allocation failed");

  ssize_t i=0;
  ssize_t check=0;
  while ((check = read(0,&buff[i],1))){
     if (check != 1){
         free(buff);
         err(-1,"Error while reading");
     }
      
     if(buff[i] == '\n') break;
        i++;
 }
 buff[i]='\0';
 return buff;
}

void pipeline(char **cmd1, char **cmd2){
  pid_t p = fork();
  if ( p == -1)
    err(-1,"Failed fork");

  if ( p == 0 ){
    int pd[2];
    if (pipe(pd) == -1)
         err(-1, "Failed pipe");
  
    pid_t pid = fork();
    if (pid == -1)
      err(-1,"Failed fork");
     
    if (pid == 0){
      //child
      if ( (close(pd[READ])) == -1)
        err(-1,"Failed close");
      
      if ( (dup2(pd[WRITE], 1)) == -1 )
        err(-1,"Failed dup2");

      if ( (close(pd[WRITE])) == -1)
        err(-1,"Failed close");

      execvp(*cmd1,cmd1); 
      err(-1,"Failed exec");
    }
                
    //parent
    if ((close(pd[WRITE])) == -1)
      err(-1,"Failed close");
    
    if ((dup2(pd[READ],0)) == -1)
      err(-1,"Failed dup");
    
    if((close(pd[READ])) == -1)
      err(-1,"Failed close");

    execvp(*cmd2, cmd2);  
    err(-1,"Failed exec");
  } 
  wait(NULL);
}

void execute_single_command(char **argv){
  pid_t pid = fork();
     
  if (pid == -1)
      err(-1,"Failed fork");

  if(pid == 0 ){
      //child
      execvp(*argv,argv);
      err(-1,"Failed exec");
  }
  //parent       
  wait(NULL);     
}

void execute_line(char *line){
  char *commands[ARG_MAX];
  split(line,commands,";");

  for(int i=0; commands[i]!=NULL;i++){
      char tmp_cmd[ARG_MAX];
      char tmp_left[ARG_MAX];
      
      char *cmd[ARG_MAX];
      char *left_cmd[ARG_MAX];
      
      strcpy(tmp_cmd,commands[i]);
      split(tmp_cmd,cmd,"|");

      if(cmd[2]!=NULL)
          errx(-1,"Failed multiple pipe");
                                         
      strcpy(tmp_left,cmd[0]);
      split(tmp_left,left_cmd," ");

      if(cmd[1] != NULL){
           char tmp_right[ARG_MAX];
           char *right_cmd[ARG_MAX];
                                                             
           strcpy(tmp_right,cmd[1]);
           split(tmp_right,right_cmd," ");

           pipeline(left_cmd,right_cmd);
      }
      else{
           execute_single_command(left_cmd);
      }
  }
}

void parallel_execution(char* line){
  tasks++;
  pid_t pid = fork();

  if( pid == -1)
    err(-1,"Failed fork");

  if (pid == 0){
    //child
    execute_line(line);
  }
  
  //parent
  if((waitpid(-1,NULL,WNOHANG)) > 0)
      tasks--;

  if(tasks == max_tasks){
     wait(NULL);
     tasks--;
  }
}

int main(int argc, char* argv[]){
  if(argc != 2)
    errx(-1,"Incorrect number of parameters");

  max_tasks=convert(argv[1]);
  char* line=read_line();

  int counter=0;

  while(strlen(line)>0){
    parallel_execution(line);
    free(line);
    line=read_line();
    counter++;
  }

  for(int i=0;i< counter;i++)
    wait(NULL); 
  
  free(line);
  return 0;
}
