#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "dynarray.h"

// Assumption: Any line doesn't cross 80 characters
#define LINELEN 80

struct file{
  char *name; 
  dynarray lines; // should be a dynamic array
};
typedef struct file *file; 

void printLine(FILE *out, DA_ELEMENT el, int n){
  // typecase the void *el into char *
  char *line = (char *)el;
  fprintf(out, "%s\n", line);
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
  file returnFile = constructFile();

  returnFile->name = name; 

  FILE *f = fopen(name, "r");
  assert(f != NULL);

  // need to read the lines 
  char line[LINELEN];
  while (fgets(line, LINELEN, f) != NULL){
    // need to remove the '/n' at the end 
    int len = strlen(line);
    if (len > 0 && line[len - 1] == '\n'){
      line[len - 1] = '\0';
    }
    // printf("line read <%s>\n", line);
    add_dynarray(returnFile->lines, strdup(line));
  }

  fclose(f);

  return returnFile; 
}

// Need to extract the definitions 
// Hacky way of doing it would be to just get the function definitions 
// which are not indented, lol :)
// need to have '(', ')' and { 
dynarray extractFns(file f){
  // iterate over the lines 

  char combined[1024] = {0};
  bool collecting = false; 

  for (int x = 0; x < f->lines->len; x++){
    char *line = f->lines->data[x];
  
    // ignore interiro and static functions 
    if (line[0] == ' '|| strstr(line, "static") != NULL){
      continue; 
    }

    // ignore comments and main function 
    if (strstr(line, "//") != NULL || strstr(line, "main") != NULL){
      continue;
    }
    
    if (strchr(line, '(')){
      collecting = true;
    }

    if (collecting){
      strcat(combined, line);
      strcat(combined, " ");
    }

    if (collecting && strchr(line, ')')){
      // check if combined looks like a function 
      if (strchr(combined, '{') || (x + 1 < f->lines->len && strchr(f->lines->data[x+1], '{'))){
        // its a function 
        // remove the trailing { if present 
        if (strchr(combined, '{')){
          int i = strlen(combined) - 1;
          for ( ; i >= 0 && combined[i] != '{'; i--){
            /* EMPTY BODY */
          }
          combined[i] = '\0';
        }
        char fun[1200] = "extern ";
        strcat(fun, combined);
        strcat(fun, ";");
        printf("%s\n", fun);
      }
      collecting = false; 
      combined[0] = '\0';
    }
  }
  return NULL;
}


int main(int argc, char **argv){
  
  if (argc != 2){
    fprintf(stdout, "Usage : jproto <fileName.c>");
    exit(EXIT_FAILURE);
  }

  // printf("%s", argv[1]);

  file f = readFile(argv[1]);
  // print_dynarray(f->lines, stdout);

  extractFns(f);

  // freeFile(f);

  return EXIT_SUCCESS;
}
