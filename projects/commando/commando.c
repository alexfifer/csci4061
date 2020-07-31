#include "commando.h"

int main(int argc, char* argv[]){
  setvbuf(stdout, NULL, _IONBF, 0); // Turn off output buffering

  int echo_flag = 0;                           // Flag for echoing user input, default is off
  int ntok;                                    // Number of tokens passed to commando
  char command_args[MAX_LINE];                 // Arguments including spaces
  char **command = malloc(NAME_MAX * ARG_MAX); // Array to hold parsed tokens

  cmdcol_t *cmdcol = (cmdcol_t *)malloc(MAX_CMDS * sizeof(cmd_t)); // Init command collection
  cmdcol->size = 0;  // For first addition to cmdcol, size must be initialized to 0

  // Check for correct command line arguments and if echo is enabled by user
  if (argv[1]){
    if (strcmp(argv[1], "--echo") == 0){
      echo_flag = 1;
    }
    else{
      printf("Invalid option '%s'\nTry '--echo' or no option.\n", argv[1]);
      exit(1);
    }
  }

  // Additionally, check environment variable for echo
  if (getenv("COMMANDO_ECHO") != NULL) {
    echo_flag = 1;
  }

  // Main loop, continues until user chooses to exit
  while(1){ 
    printf("@> ");
    if(!fgets(command_args, MAX_LINE, stdin)){
      printf("\nEnd of input\n");
      break;
    }

    parse_into_tokens(command_args, command, &ntok);

    if(echo_flag) {
      for (int i = 0; i < ntok; i++){
        printf("%s ", command[i]);
      }
      printf("\n");
    }

    if (command[0] == NULL){} // user hit 'enter', looping continues

    else if(strcmp("help", command[0]) == 0){
      printf("COMMANDO COMMANDS\n"
             "help               : show this message\n"
             "exit               : exit the program\n"
             "list               : list all jobs that have been started giving information on each\n"
             "pause nanos secs   : pause for the given number of nanseconds and seconds\n"
             "output-for int     : print the output for given job number\n"
             "output-all         : print output for all jobs\n"
             "wait-for int       : wait until the given job number finishes\n"
             "wait-all           : wait for all jobs to finish\n"
             "command arg1 ...   : non-built-in is run as a job\n");
    }

    else if(strcmp("exit", command[0]) == 0){
      break;
    }

    // list all jobs that have been started giving information on each
    else if(strcmp("list", command[0]) == 0){
      cmdcol_print(cmdcol);
    }

    // pause for the given number of nanseconds and seconds
    else if(strcmp("pause", command[0]) == 0){
      pause_for(atol(command[1]), atoi(command[2]));
      cmdcol_update_state(cmdcol, NOBLOCK);
    }

    // print the output for a given job number
    else if(strcmp("output-for", command[0]) == 0){
      int job_num = atoi(command[1]);
      if (job_num < 0 || job_num >= cmdcol->size){
        printf("no such job");
      }
      else {
        cmd_t *job = cmdcol->cmd[job_num]; // pointer to the job, a cmd_t struct
        printf("@<<< Output for %s[#%d] (%d bytes):\n", job->name, job->pid, job->output_size);
        printf("----------------------------------------\n");
        cmd_print_output(job);
        printf("----------------------------------------\n");
      }

    }

    // print the output for all jobs
    else if(strcmp("output-all", command[0]) == 0){
      for (int j = 0; j < cmdcol->size; j++){
        printf("@<<< Output for %s[#%d] (%d bytes):\n", cmdcol->cmd[j]->name,
	                                                      cmdcol->cmd[j]->pid,
                                                        cmdcol->cmd[j]->output_size);
        printf("----------------------------------------\n");
        cmd_print_output(cmdcol->cmd[j]);
        printf("----------------------------------------\n");
      }
    }

    // wait until the given job number finishes
    else if(strcmp("wait-for", command[0]) == 0){
      int job_wait = atoi(command[1]);
      if (job_wait < 0 || job_wait >= cmdcol->size){
        printf("no such job");
      }
      else {
        cmd_update_state(cmdcol->cmd[job_wait], DOBLOCK);
      }
    }

    // wait for all jobs to finish
    else if(strcmp("wait-all", command[0]) == 0){
      for (int i = 0; i < cmdcol->size; i++){      // cycle through all jobs in command collection
        cmd_update_state(cmdcol->cmd[i], DOBLOCK); // update command state to blocking
      }
    }

    // non-built-in is run as a job
    else {
      cmd_t *new_job = cmd_new(command); // user entered non-commando command, so fork new process
      cmd_start(new_job);
      cmdcol_add(cmdcol, new_job);
    }
    cmdcol_update_state(cmdcol, NOBLOCK); // update commands in collection each loop iteration
  }

  // deallocate everything to prevent memory leak
  free(command);
  cmdcol_freeall(cmdcol);
  free(cmdcol);
  return 0;
}
