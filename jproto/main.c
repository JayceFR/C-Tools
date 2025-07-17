#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "dynarray.h"

struct file{
  char *name; 
  dynarray lines; // should be a dynamic array
};
typedef struct file *file; 

void printLine(FILE *out, DA_ELEMENT el, int n){
  // typecase the void *el into char *
  char *line = (char *)el;
  fprintf(out, line);
}

void freeLine(DA_ELEMENT el){
  char *line = (char *) el; 
  free(line);
}

file constructFile(){
  file newFile = malloc(sizeof(struct file));
  assert(newFile != NULL);

  newFile->lines = create_dynarray(&freeLine, &printLine);

  return newFile;
}

void freeFile(file f){
  free(f->name);
  free_dynarray(f->lines);
  free(f);
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
