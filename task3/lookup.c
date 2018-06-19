#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

char* read_to_symbol(ssize_t file, const char delim){
   ssize_t size=256;
   char* buff=(char*)malloc(size);
   if (buff == NULL ){
        const int savederrorcode = errno;
        close(file);
        errno = savederrorcode;
        err(-1,"Memory allocation failed");
   }

   ssize_t i=0;
   ssize_t check=0;
   while ((check = read(file,&buff[i],1))){
     if (check != 1){ 
        const int savederrorcode = errno;
        close(file);
        free(buff);
        errno = savederrorcode;
        err(-1,"Error while reading");
     }

     if(buff[i] == delim) break;   
     i++;
     if(i == size){
        size*=2;
        buff = (char*)realloc(buff,size);
      }
   }

   buff[i]='\0';
   return buff;
}

void move_to_null(ssize_t file){
  char symb;
  if ( read(file,&symb,1) != 1 ){
        const int savederrorcode = errno;
        close(file);
        errno = savederrorcode;
        err(-1,"Error while reading");
   }

  if( symb == '\0')
    return;

  off_t offset=lseek(file,0,SEEK_CUR);
  offset--;
  lseek(file,offset,SEEK_SET);

  if (read(file,&symb,1) != 1 ){
        const int savederrorcode = errno;
        close(file);
        errno = savederrorcode;
        err(-1,"Error while reading");
   }

  while( symb != '\0' ){
      lseek(file,offset-1,SEEK_SET);
      if ( read(file,&symb,1) != 1 ){
          const int savederrorcode = errno;
          close(file);
          errno = savederrorcode;
          err(-1,"Failed while reading file");
      }
      offset--;
  }
}

char* get_word(ssize_t file){
  move_to_null(file);
  char* buff=read_to_symbol(file,'\n');
  return buff;
}

ssize_t print_str(char* str){
  ssize_t len = strlen(str);
  if ( (write(1,str,len)) == len )
    return len;
  else
    return -1;
}

int main(int argc, char* argv[])
{
    if(argc != 3){
        errx(-1,"Invalid number of arguments");
        return 1;
     }

    ssize_t fd=open(argv[1],O_RDONLY);
    if(fd == -1){
       err(-1,"Failed open file");
     }
   
    char *word = argv[2];
      
    lseek(fd,1,SEEK_SET);
    char* left_str = read_to_symbol(fd, '\n');
    if( strcmp(word,left_str) == 0){  
        
        char* meaning=read_to_symbol(fd,'\0');
        if ( (print_str(meaning)) == -1 ){
          const int savederrorcode = errno;
          free(meaning); 
          free(left_str);
          close(fd);
          errno = savederrorcode;
          err(-1,"Failed while writing");
        }

        free(meaning); 
        free(left_str);     
        close(fd);
        return 0;
    }
      
    lseek(fd,-2,SEEK_END);
    char* right_str = get_word(fd);
    if( strcmp(word,right_str) == 0){  
        
        char* meaning=read_to_symbol(fd,'\0');
        if ( (print_str(meaning)) == -1 ){
          const int savederrorcode = errno;
          free(meaning);
          free(left_str);
          free(right_str);
          close(fd);
          errno = savederrorcode;
          err(-1,"Failed while writing");
       }

      free(meaning);
      free(left_str);
      free(right_str);     
      close(fd);
      return 0;
    }
 
    if( strcmp(word, left_str ) < 0 || strcmp(word,right_str) > 0 ){
          close(fd);
          free(left_str);
          free(right_str);
          errx(-1,"Not a such word in the dictionary");
    }

    free(left_str);
    free(right_str);

    off_t n = lseek(fd,0,SEEK_END);
    off_t left=0;
    off_t right=n-1;
    off_t middle; 
    char* curr_str;

    while( right - left > 1 ){
        middle = ( left + right ) / 2; 

        lseek(fd,middle,SEEK_SET);
        curr_str = get_word(fd);
        
       
        if( strcmp(word, curr_str ) == 0 ){
          char* meaning=read_to_symbol(fd,'\0');
          
          if ( (print_str(meaning)) == -1 ){
          const int savederrorcode = errno;
          free(meaning);
          free(curr_str);
          close(fd);
          errno = savederrorcode;
          err(-1,"Failed while writing");
          }
         
          free(meaning);
          free(curr_str);
          return 0;
        }

        if( strcmp(word, curr_str) < 0 )
          right = middle;
        else
          left = middle;
      
        free(curr_str);
     }

      close(fd);
      errx(-1,"Not a such word in the dictionary");
}

