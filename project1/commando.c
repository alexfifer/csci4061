#include "commando.h"

int main(int argc, char* argv[]){
  setvbuf(stdout, NULL, _IONBF, 0); // Turn off output buffering
    
  int echo_set = 0;
  int tokens;

  char command_args[MAX_LINE];
  char **command = malloc(NAME_MAX * ARG_MAX);
  /*
  if (argc >= 2 && strcmp(argv[1], "--echo")==0){
    echo_set = 1;
  }
  **/
  
  while(1){
    printf("@> ");
    if(!fgets(command_args, MAX_LINE, stdin)){
      printf("\nEnd of input\n");
      break;
    }

    parse_into_tokens(command_args, command, &tokens);

    if (command[0] == NULL){}

    else if(strcmp("help", command[0]) == 0){
      printf("\nCOMMANDO COMMANDS\n");
      printf("help               : show this message\n");
      printf("exit               : exit the program\n");
      printf("list               : list all jobs that have been started giving information on each\n");
      printf("pause nanos secs   : pause for the given number of nanseconds and seconds\n");
      printf("output-for int     : print the output for given job number\n");
      printf("output-all         : print output for all jobs\n");
      printf("wait-for int       : wait until the given job number finishes\n");
      printf("wait-all           : wait for all jobs to finish\n");
      printf("command arg1 ...   : non-built-in is run as a job\n");
    }

    else if(strcmp("exit", command[0]) == 0){
      break;
    }
    else if(strcmp("list", command[0]) == 0){
      break;
    }
    else if(strcmp("pause", command[0]) == 0){
      break;
    }
    else if(strcmp("output-for", command[0]) == 0){
      break;
    }
    else if(strcmp("output-all", command[0]) == 0){
      break;
    }
    else if(strcmp("wait-for", command[0]) == 0){
      break;
    }
    else if(strcmp("wait-all", command[0]) == 0){
      break;
    }
    else {
      //call to cmd_new
    }
  }
  
  //free calls
  free(command);
  return 0;
}
