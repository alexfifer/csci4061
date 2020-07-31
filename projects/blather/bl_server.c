#include "blather.h"

int running;                                           // global running flag

void handle_signal(int sig) {                          // in case of interrupt, shutdown server
  running = 0;
}

int main(int argc, char **argv) {

  if(argc < 2) {
    printf("usage: %s <server-name>\n", argv[0]);
    return 1;
  }

  struct sigaction sa = {.sa_handler = handle_signal}; // init signal handler
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);

  server_t server = {};                                // init server
  server_start(&server, argv[1], DEFAULT_PERMS);
  running = 1;
  int i;
  while (running) {
    server_check_sources(&server);
    if (server_join_ready(&server))
      server_handle_join(&server);
    for (i = 0; i < server.n_clients; i++) {           // TODO hacky way to do this?
      if (server_client_ready(&server, i))             //      lot of calls to server.n_clients
        server_handle_client(&server, i);
    }
  }
  server_shutdown(&server);                            // gracefully end the server process
  return 0;
}
