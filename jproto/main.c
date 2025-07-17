#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "dynarray.h"

// Assumption: Any line doesn't cross 80 characters
#define LINELEN 180

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

file constructFile(void){
  file newFile = malloc(sizeof(struct file));
  assert(newFile != NULL);

  newFile->lines = create_dynarray(&freeLine, &printLine);

  return newFile;
}

void freeFile(file f){
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

  dynarray funs = create_dynarray(&freeLine, &printLine);

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
        add_dynarray(funs, strdup(fun));
      }
      collecting = false; 
      combined[0] = '\0';
    }
  }
  return funs;
}

void uppercase_basename(const char *path, char *up){
  const char *base = strchr(path, '/');
  if (!base) base = path;
  else base++; // skip the '/'

  int i; 

  for (i = 0; base[i] && base[i] != '.' && i < sizeof(up) - 1; i++){
    up[i] = toupper(base[i]);
  }

  up[i] = '\0';
  
}

void writeUpdateHeaders(const char *header_name, dynarray prototypes, int header_len){
  FILE *f = fopen(header_name, "r");
  dynarray lines = create_dynarray(freeLine, printLine);

  char guard[128];
  char upHeader[header_len];
  uppercase_basename(header_name, upHeader);
  snprintf(guard, sizeof(guard), "__%s_H__", upHeader);

  if (f){
    // File exists 
    char line[LINELEN];
    while (fgets(line, sizeof(line), f)){
      add_dynarray(lines, strdup(line));
    }
    fclose(f);
  } else{
    // Create a fake one  
    char line[150];   

    sprintf(line, "#ifndef %s \n", guard);
    printf("%s\n", line);
    add_dynarray(lines, strdup(line));

    sprintf(line, "#define %s \n", guard);
    printf("%s\n", line);
    add_dynarray(lines, strdup(line));

    sprintf(line, "#endif // %s \n\n ", guard);
    printf("%s\n", line);
    add_dynarray(lines, strdup(line));
  }

  // list of flags for each prototype, which says if it is written or not 
  bool *flags = malloc(prototypes->len * sizeof(bool));
  for (int i = 0; i < prototypes->len; i++){
    flags[i] = false;
  }

  // Mark the prototypes visited 
  for (int i = 0; i < prototypes->len; i++){
    char *fun = prototypes->data[i];
    for (int j = 0; j < lines->len; j++){
      if (strstr(lines->data[j], fun)){
        flags[i] = true;
      }
    }
  }
  
  // Now insert the non visited prototypes before the #endif 
  int endifPos = lines->len - 1;
  for (; endifPos >= 0 && strstr(lines->data[endifPos], "#endif") == NULL; endifPos--){
    /* EMPTY BODY */
  }

  f = fopen(header_name, "w");
  assert(f != NULL);

  // write everything upto endif pos 
  for (int i = 0; i < endifPos; i++){
    fprintf(f, "%s", (char *) lines->data[i]);
  }

  // now write the missing prototypes 
  for (int i = 0; i < prototypes->len; i++){
    if (!flags[i]){
      fprintf(f, "%s\n", (char *) prototypes->data[i]);
    }
  }

  fprintf(f, "\n#endif // %s\n", guard);

  free(flags);

  fclose(f);
  free_dynarray(lines);
}

int main(int argc, char **argv){
  
  if (argc != 2){
    fprintf(stdout, "Usage : jproto <fileName.c>");
    exit(EXIT_FAILURE);
  }

  // printf("%s", argv[1]);

  file f = readFile(argv[1]);
  // print_dynarray(f->lines, stdout);

  dynarray funs = extractFns(f);
  print_dynarray(funs, stdout);

  char header[80] = {0};
  strcpy(header, f->name);
  // replace the trailing c with h 
  int i = strlen(header);
  for (; i >= 0 && header[i] != 'c'; i--){
    /* EMPTY BODY */
  }
  header[i] = 'h';

  writeUpdateHeaders(header, funs, strlen(header));

  free_dynarray(funs);
  freeFile(f);

  return EXIT_SUCCESS;
}
