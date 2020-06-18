// collatze_test.c: testing program for functions in collatz_funcs.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "collatz.h"

int main(int argc, char **argv){
  char *progname = argv[0];

  if(argc < 3){
    printf("usage: %s next <int>\n",progname);
    printf("usage: %s steps <int> {0,1}\n",progname);
    printf("Test a function from collatz.c\n");
    return 1;
  }

  char *mode = argv[1];
  int start = atoi(argv[2]);

  if( strcmp(mode,"next")==0 ){
    printf("running collatz_next(%d)\n",start);
    int ret = collatz_next(start);
    printf("returned: %d\n",ret);
  }
  else if( strcmp(mode,"steps")==0 ){
    if(argc < 4){
      printf("usage: %s steps <int> {0,1}\n",progname);
      printf("steps requires another command line argument: 0 or 1\n");
      return 1;
    }
    int print_output = atoi(argv[3]);
    printf("running collatz_steps(%d, %d)\n",start,print_output);
    int ret = collatz_steps(start, print_output);
    printf("returned: %d\n",ret);
  }
  else{
    printf("ERROR: unknown mode '%s', exiting\n",mode);
    return 1;
  }

  return 0;
}
