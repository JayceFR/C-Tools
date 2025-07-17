#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct file{
  char *name; 
  char *lines; // should be a dynamic array
  int size; 
};
typedef struct file *file; 

file constructFile(){
  file newFile = malloc(sizeof(struct file));
  assert(newFile != NULL);



  return newFile;
}

// Need to open and read the file and return the lines 

file readFile(char *name){

}


int main(int argc, char **argv){
  
  if (argc != 1){
    fprintf(stdout, "Usage : jproto <fileName.c>");
    exit(EXIT_FAILURE);
  }




  return EXIT_SUCCESS;
}
