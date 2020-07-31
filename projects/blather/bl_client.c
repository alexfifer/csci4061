#include "blather.h"

simpio_t simpio_actual;
simpio_t *simpio = &simpio_actual;

client_t client_actual;
client_t *client = &client_actual;

pthread_t user_thread;
pthread_t background_thread;

join_t request;                                        // chat room request for both threads to use

// worker thread to manage user input
void *user_worker(void *arg) {
  int fd = open(client->to_server_fname, O_RDWR);
  check_fail(fd < 0, 1, "Error opening to-server FIFO\n");
  while (!simpio->end_of_input) {
    simpio_reset(simpio);
    iprintf(simpio, "");
    while (!simpio->line_ready && !simpio->end_of_input) {        // read until line is complete
      simpio_get_char(simpio);
    }
    mesg_t mesg = {};
    strcpy(mesg.body, simpio->buf);
    strcpy(mesg.name, client->name);                   
    if (simpio->line_ready)
      mesg.kind = BL_MESG;
    else
      mesg.kind = BL_DEPARTED;
    int nwrite = write(fd, &mesg, sizeof(mesg_t));
    check_fail(nwrite < 0, 1, "Error writing message to server FIFO\n");
  }
  pthread_cancel(background_thread);                   // kill the background thread
  return NULL;
}

// worker thread to listen to the info from the server
void *background_worker(void *arg) {
  int fd = open(client->to_client_fname, O_RDWR);
  check_fail(fd < 0, 1, "Error opening to-client FIFO\n"); 
  while (1) {
    mesg_t mesg = {};
    int nread = read(fd, &mesg, sizeof(mesg_t));
    check_fail(nread < 0, 1, "Error reading to-client FIFO\n");
    if (mesg.kind == BL_JOINED) {
      iprintf(simpio, "-- %s JOINED --\n", mesg.name);
    }
    else if (mesg.kind == BL_DEPARTED) {
      iprintf(simpio, "-- %s DEPARTED --\n", mesg.name);
    }
    else if (mesg.kind == BL_MESG) {
      iprintf(simpio, "[%s] : %s\n", mesg.name, mesg.body);
    }
    else if (mesg.kind == BL_SHUTDOWN) {
      iprintf(simpio, "!!! server is shutting down !!!\n");
      break;
    }
  }
  pthread_cancel(user_thread);                         // kill the user thread
  return NULL;
}

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("usage: ./blclient <server> <user name>\n");
    return (0);
  }

  // prepare file paths for FIFOs
  char to_client[MAXNAME];
  char to_server[MAXNAME];
  char to_join[MAXNAME];
  snprintf(to_client, MAXNAME, "client_%d.fifo", getpid());
  snprintf(to_server, MAXNAME, "server_%d.fifo", getpid());
  snprintf(to_join, MAXNAME, "%s.fifo", argv[1]);

  // prepare join request
  strcpy(request.name, argv[2]);
  strcpy(request.to_client_fname, to_client);
  strcpy(request.to_server_fname, to_server);

  // prepare client
  strcpy(client->name, argv[2]);
  strcpy(client->to_client_fname, to_client);
  strcpy(client->to_server_fname, to_server);

  // establish client/server connection
  check_fail(mkfifo(to_client, DEFAULT_PERMS) < 0, 1, "Make to-client FIFO error\n");
  check_fail(mkfifo(to_server, DEFAULT_PERMS) < 0, 1, "Make to-server FIFO error\n");
  
  // request to join the server
  int join_fd = open(to_join, O_WRONLY, DEFAULT_PERMS);
  check_fail(join_fd < 0, 1, "Error opening request FIFO\n");
  int nwrite = write(join_fd, &request, sizeof(join_t));
  check_fail(nwrite < 0, 1 , "Error writing request to request FIFO\n");
  
  // init simplified terminal I/O
  char prompt[MAXNAME];                                // display prompt with client's name
  strncpy(prompt, argv[2], MAXNAME);
  strncat(prompt, PROMPT, MAXNAME - strlen(PROMPT));   // null terminate prompt string
  simpio_set_prompt(simpio, prompt);
  simpio_reset(simpio);
  simpio_noncanonical_terminal_mode();
  
  // start threads
  pthread_create(&user_thread, NULL, user_worker, NULL);
  pthread_create(&background_thread, NULL, background_worker, NULL);

  // join threads
  pthread_join(user_thread, NULL);
  pthread_join(background_thread, NULL);
  
  // reset to original terminal mode
  simpio_reset_terminal_mode();
  printf("\n");
  return 0;
}
