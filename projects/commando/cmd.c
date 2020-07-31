#include "commando.h" // provides required structs, defines and prototypes

// cmd.c: functions related the cmd_t struct abstracting a
// command. Most functions maninpulate cmd_t structs.

// Allocates a new cmd_t with the given argv[] array. Makes string
// copies of each of the strings contained within argv[] using
// strdup() as they likely come from a source that will be
// altered. Ensures that cmd->argv[] is ended with NULL. Sets the name
// field to be the argv[0]. Sets finished to 0 (not finished yet). Set
// str_status to be "INIT" using snprintf(). Initializes the remaining
// fields to obvious default values such as -1s, and NULLs.
cmd_t *cmd_new(char *argv[]){
  // allocate new cmd_t
  cmd_t *cmd = malloc(sizeof(cmd_t));
  // copy argv[] strings into new cmd_t
  int i = 0;
  while(argv[i] != NULL){
    cmd->argv[i] = strdup(argv[i]); // strdup() allocates memory for the duplication
    i++;
  }
  cmd->argv[i] = NULL; // null terminate argv
  // init remaining fields in cmd_t struct definition
  strcpy(cmd->name, cmd->argv[0]);
  cmd->pid = -1;
  cmd->out_pipe[PREAD] = cmd->out_pipe[PWRITE] = -1;
  cmd->finished = 0;
  cmd->status = -1;
  snprintf(cmd->str_status, STATUS_LEN, "INIT"); // this returns an uneeded integer
  cmd->output = NULL; // might already be assigned NULL
  cmd->output_size = -1;
  return cmd;
}

// Deallocates a cmd structure. Deallocates the strings in the argv[]
// array. Also deallocats the output buffer if it is not
// NULL. Finally, deallocates cmd itself.
void cmd_free(cmd_t *cmd){
  if(cmd != NULL){
    // deallocate argv[] contents
    for(int i=0; cmd->argv[i] != NULL; i++){
      free(cmd->argv[i]);
    }
    // deallocate output buffer and rest of cmd struct
    if(cmd->output != NULL)
      free(cmd->output);
    free(cmd);
  }
}

// Forks a process and starts executes command in cmd in the process.
// Changes the str_status field to "RUN" using snprintf().  Creates a
// pipe for out_pipe to capture standard output.  In the parent
// process, ensures that the pid field is set to the child PID. In the
// child process, directs standard output to the pipe using the dup2()
// command. For both parent and child, ensures that unused file
// descriptors for the pipe are closed (write in the parent, read in
// the child).
void cmd_start(cmd_t *cmd){
  int pipe_result = pipe(cmd->out_pipe);          // create a pipe
  if(pipe_result != 0){
    perror("Failed to create pipe");
    exit(1);
  }
  snprintf(cmd->str_status, 4, "RUN");            // update status
  if((cmd->pid = fork()) < 0){                    // create a child process
    perror("Failed to fork");
    exit(1);
  }
  if(cmd->pid == 0){                              // CHILD
    close(cmd->out_pipe[PREAD]);                  // close read end of pipe
    dup2(cmd->out_pipe[PWRITE], STDOUT_FILENO);   // child's standard output enters pipe
    execvp(cmd->name, cmd->argv);                 // execute command
    perror("Execution error");                    // this should be unreachable!
    exit(1);
  }                                               // PARENT
  close(cmd->out_pipe[PWRITE]);                   // close write end of pipe
}

// If cmd->finished is zero, prints an error message with the format
// 
// ls[#12341] not finished yet
// 
// Otherwise retrieves output from the cmd->out_pipe and fills
// cmd->output setting cmd->output_size to number of bytes in
// output. Makes use of read_all() to efficiently capture
// output. Closes the pipe associated with the command after reading
// all input.
void cmd_fetch_output(cmd_t *cmd){
  if(cmd->finished == 0){
    printf("%s[%d] not finished yet\n", cmd->name, cmd->pid);
  }
  else{
    int nbytes;
    cmd->output = read_all(cmd->out_pipe[PREAD], &nbytes);   // Retrieve output from pipe
    cmd->output_size = nbytes;                               // Update amount of bytes read
    close(cmd->out_pipe[PREAD]);                             // Close pipe and finish
  }
}

// Prints the output of the cmd contained in the output field if it is
// non-null. Prints the error message
// 
// ls[#17251] : output not ready
//
// if output is NULL. The message includes the command name and PID.
void cmd_print_output(cmd_t *cmd){
  if(cmd->output == NULL){
    printf("%s[#%d] : output not ready\n", cmd->name, cmd->pid);
  }
  else{
    printf("%s\n", (char*)cmd->output);
  }
}

// If the finished flag is 1, does nothing. Otherwise, updates the
// state of cmd.  Uses waitpid() and the pid field of command to wait
// selectively for the given process. Passes block (one of DOBLOCK or
// NOBLOCK) to waitpid() to cause either non-blocking or blocking
// waits.  Uses the macro WIFEXITED to check the returned status for
// whether the command has exited. If so, sets the finished field to 1
// and sets the cmd->status field to the exit status of the cmd using
// the WEXITSTATUS macro. Calls cmd_fetch_output() to fill up the
// output buffer for later printing.
//
// When a command finishes (the first time), prints a status update
// message of the form
//
// @!!! ls[#17331]: EXIT(0)
//
// which includes the command name, PID, and exit status.
void cmd_update_state(cmd_t *cmd, int block){
  if(!cmd->finished){
    int status;
    pid_t pid = waitpid(cmd->pid, &status, block);
    if(pid == cmd->pid){
      if(WIFEXITED(status)){
        cmd->status = WEXITSTATUS(status);
        snprintf(cmd->str_status, STATUS_LEN, "EXIT(%d)", cmd->status);
        cmd->finished = 1;
        cmd_fetch_output(cmd);
        printf("@!!! %s[#%d]: %s\n", cmd->name, cmd->pid, cmd->str_status);
      }
      else{
        perror("incorrect termination status");
        exit(1);
      }
    }
    else if(pid < 0){
      perror("returned incorrect pid");
      exit(1);
    }
  }
}

// Reads all input from the open file descriptor fd. Stores the
// results in a dynamically allocated buffer which may need to grow as
// more data is read.  Uses an efficient growth scheme such as
// doubling the size of the buffer when additional space is
// needed. Uses realloc() for resizing.  When no data is left in fd,
// sets the integer pointed to by nread to the number of bytes read
// and return a pointer to the allocated buffer. Ensures the return
// string is null-terminated. Does not call close() on the fd as this
// is done elsewhere.
char *read_all(int fd, int *nread){
  int bpos = 0;                                     // position in buffer
  int nbytes = 0;                                   // number of bytes read into buffer
  int bsize = BUFSIZE;
  char *buf = malloc(bsize*sizeof(char));           // allocate input buffer
  while(1){
    if(bpos >= bsize){
      bsize = bsize * 2;                            // double buffer size
      buf = realloc(buf, bsize);                    // reallocate buffer to double size
      if(buf == NULL){
        perror("could not expand read buffer");
        exit(1);
      }
    }
    int read_max = bsize - bpos;                    // calculate max amount to be read
    int read_amount = read(fd, buf+bpos, read_max); // perform read
    nbytes += read_amount;                          // update amount read
    if(read_amount == 0){
      *nread = nbytes;                              // set int pointer to amount read on success
      break;                                        // exit the loop if reached end of input
    }
    else if(read_amount == -1){
      perror("read failed");
      exit(1);
    }
    bpos += read_amount;                            // read was success if we made it this far
  }
  buf[nbytes] = '\0';                               // null terminate buffer
  return buf;
}
