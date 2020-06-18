// collatz_main.c: main function to interactively run collatz
// computations from collatz.c interactively.

#include <stdio.h>
#include <string.h>
#include "collatz.h"

int main(int argc, char **argv){
  int echo_input = 0;           // controls whether input is echoed
  if(argc >= 2 && strcmp(argv[1], "-echo")==0){
    echo_input=1;               // turn echoing on, likely for automated testing
  }

  int nstart;
  printf("Enter the starting integer:\n");
  printf(">> ");
  scanf("%d", &nstart);
  if(echo_input){
    printf("%d\n",nstart);
  }

  int nnext = collatz_next(nstart);
  printf("The next value in the Collatz sequence is %d\n",nnext);

  int verbose;
  printf("Show output of steps (0:NO, any other int: yes):\n");
  printf(">> ");
  scanf("%d", &verbose);
  if(echo_input){
    printf("%d\n",verbose);
  }

  int steps = collatz_steps(nstart,verbose);
  printf("The starting value %d converged to 1 in %d steps\n",
         nstart,steps);
  return 0;    
}

