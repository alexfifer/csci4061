#include "blather.h"

client_t *server_get_client(server_t *server, int idx) {
// Gets a pointer to the client_t struct at the given index. If the
// index is beyond n_clients, the behavior of the function is
// unspecified and may cause a program crash.
  dbg_printf("Retrieving client at index [%d]\n", idx);
  return &server->client[idx];
}

void server_start(server_t *server, char *server_name, int perms) {
// Initializes and starts the server with the given name. A join fifo
// called "server_name.fifo" should be created. Removes any existing
// file of that name prior to creation. Opens the FIFO and stores its
// file descriptor in join_fd.
  log_printf("BEGIN: server_start()\n");
  strncpy(server->server_name, server_name, MAXPATH);  // init server struct
  server->join_ready = 0;
  server->n_clients = 0;
  char fifo_path[MAXPATH];                             // create file path for server fifo
  sprintf(fifo_path, "%s.fifo", server_name);
  dbg_printf("Server initialized join FIFO with path '%s'\n", fifo_path);
  remove(fifo_path);                                   // remove old requests FIFO
  mkfifo(fifo_path, perms);                            // create server FIFO
  int join_fd = -1;
  check_fail(((join_fd = open(fifo_path, O_RDWR)) < 0), 1, "Invalid FIFO path");
  dbg_printf("Server initiliazed join FIFO with file descriptor %d\n", join_fd);
  server->join_fd = join_fd;
  log_printf("END: server_start()\n");
}

void server_shutdown(server_t *server) {
// Shut down the server. Close the join FIFO and unlink (remove) it so
// that no further clients can join. Send a BL_SHUTDOWN message to all
// clients and proceed to remove all clients in any order.
  log_printf("BEGIN: server_shutdown()\n");
  close(server->join_fd);                              // close the join FIFO
  char fifo_path[MAXPATH];
  strncpy(fifo_path, server->server_name, MAXPATH);
  strncat(fifo_path, ".fifo", MAXPATH - strlen(".fifo")); // null terminate FIFO path
  remove(fifo_path);
  mesg_t mesg = {.kind = BL_SHUTDOWN};                 // send shutdown message to clients
  server_broadcast(server, &mesg);
  int ret = 0;                                         // remove all clients from server
  int n = server->n_clients;
  for (int i=0; i < n; i++) {
    ret = server_remove_client(server, i);
    check_fail(ret < 0, 0, "Server failed to remove client\n");
  }
  log_printf("END: server_shutdown()\n");
}

int server_add_client(server_t *server, join_t *join) {
// Adds a client to the server according to the parameter join which
// should have fields such as name filled in. The client data is
// copied into the client[] array and file descriptors are opened for
// its to-server and to-client FIFOs. Initializes the data_ready field
// for the client to 0. Returns 0 on success and non-zero if the
// server as no space for clients (n_clients == MAXCLIENTS).
  log_printf("BEGIN: server_add_client()\n");
  int n = server->n_clients;
  if (n >= MAXCLIENTS)
    return 1;
  server->n_clients = n + 1;
  client_t *client = server_get_client(server, n);
  // copy client data from join request
  strncpy(client->name, join->name, MAXPATH);
  strncpy(client->to_client_fname, join->to_client_fname, MAXPATH);
  strncpy(client->to_server_fname, join->to_server_fname, MAXPATH);
  // debug info for client being added
  dbg_printf("Added client '%s' to server at index [%d], using FIFOs '%s' and '%s'\n",
             client->name, n, client->to_client_fname, client->to_server_fname);
  // establish I/O
  client->to_client_fd = open(client->to_client_fname, O_RDWR, DEFAULT_PERMS); 
  check_fail(client->to_client_fd < 0, 1, "Invalid FIFO path");
  client->to_server_fd = open(client->to_server_fname, O_RDONLY, DEFAULT_PERMS); 
  check_fail(client->to_server_fd < 0, 1, "Invalid FIFO path");
  client->data_ready = 0;
  log_printf("END: server_add_client()\n");
  return 0;
}

int server_remove_client(server_t *server, int idx) {
// Remove the given client likely due to its having departed or
// disconnected. Close fifos associated with the client and remove
// them.  Shift the remaining clients to lower indices of the client[]
// preserving their order in the array; decreases n_clients.
  dbg_printf("BEGIN: server_remove_client()\n");
  client_t *client = server_get_client(server, idx);
  dbg_printf("Removing client '%s' at index [%d]\n", client->name, idx);
  close(client->to_client_fd);
  close(client->to_server_fd);
  remove(client->to_client_fname);
  remove(client->to_server_fname);
  int n = server->n_clients;
  int i;
  for (i = idx; i < n; i++) {                          // shift remaining clients
    if (i < n - 1)
      server->client[i] = server->client[i+1];
    else                                               // if last to be shifted
      server->client[i] = (client_t) {0};              // set to null client
  }
  server->n_clients = n - 1;
  dbg_printf("END: server_remove_client()\n");
  return 0;
}

int server_broadcast(server_t *server, mesg_t *mesg) {
// Send the given message to all clients connected to the server by
// writing it to the file descriptors associated with them.
  dbg_printf("BEGIN: server_broadcast()\n");
  int i, n, ret;
  client_t *client;
  n = server->n_clients;
  dbg_printf("Server broadcasting to %d clients, message type %d\n", n, mesg->kind);
  for (i = 0; i < n; i++) {
    client = server_get_client(server, i); 
    ret = write(client->to_client_fd, mesg, sizeof(mesg_t));
    check_fail(ret < 0, 1, "Failed to write at to-client FIFO");
  }
  dbg_printf("END: server_broadcast()\n");
  return 0;
}

void server_check_sources(server_t *server) {
// Checks all sources of data for the server to determine if any are
// ready for reading. Sets the servers join_ready flag and the
// data_ready flags of each of client if data is ready for them.
// Makes use of the poll() system call to efficiently determine
// which sources are ready.
  log_printf("BEGIN: server_check_sources()\n");
  int i, n;
  n = server->n_clients;
  struct pollfd pfds[n + 1];                           // init polling for clients and join queue
  client_t *client;
  for (i = 0; i <= n; i++) {
    pfds[i].events = POLLIN;
    pfds[i].revents = 0;
    if (i < n) {
      client = server_get_client(server, i);
      pfds[i].fd = client->to_server_fd;
    }
    else {
      pfds[i].fd = server->join_fd;
    }
  }
  log_printf("poll()'ing to check %d input sources\n", n + 1);
  int ret = poll(pfds, n + 1, -1);
  log_printf("poll() completed with return value %d\n", ret);
  if (ret < 0) {
    log_printf("poll() interrupted by a signal\n");
  }
  else {
    int ready = (pfds[n].revents & POLLIN);            // check join queue for data
    server->join_ready = ready;
    log_printf("join_ready = %d\n", ready);
    for (i = 0; i < n; i++) {                            // check clients for data
      int data_ready = pfds[i].revents & POLLIN;
      client = server_get_client(server, i);
      client->data_ready = data_ready;
      log_printf("client %d '%s' data_ready = %d\n", i, client->name, data_ready);       
    }  
  }
  log_printf("END: server_check_sources()\n");
}

int server_join_ready(server_t *server) {
// Return the join_ready flag from the server which indicates whether
// a call to server_handle_join() is safe.
  return server->join_ready;
}

int server_handle_join(server_t *server) {
// Call this function only if server_join_ready() returns true. Read a
// join request and add the new client to the server. After finishing,
// set the servers join_ready flag to 0.
  log_printf("BEGIN: server_handle_join()\n");
  join_t request;
  int nread = read(server->join_fd, &request, sizeof(join_t));
  check_fail(nread < 0, 1, "Unable to read from join_fd\n");
  log_printf("join request for new client '%s'\n", request.name);
  if (server_add_client(server, &request) != 0) {      // check if server is full
    dbg_printf("Unable to add new client\n");
    return 1;
  }
  mesg_t mesg = {.kind = BL_JOINED};                   // broadcast join
  strncpy(mesg.name, request.name, MAXNAME);
  server_broadcast(server, &mesg);
  server->join_ready = 0;                              // reset join_ready flag
  log_printf("END: server_handle_join()\n");
  return 0;  
}

int server_client_ready(server_t *server, int idx) {
// Return the data_ready field of the given client which indicates
// whether the client has data ready to be read from it.
  int ready = server_get_client(server, idx) -> data_ready;
  dbg_printf("Client at index %d is ready: %d\n", idx, ready);
  return ready;
}

int server_handle_client(server_t *server, int idx) {
// Process a message from the specified client. This function should
// only be called if server_client_ready() returns true. Read a
// message from to_server_fd and analyze the message kind. Departure
// and Message types should be broadcast to all other clients.  Ping
// responses should only change the last_contact_time below. Behavior
// for other message types is not specified. 
  log_printf("BEGIN: server_handle_client()\n");
  mesg_t mesg = {};
  client_t *client = server_get_client(server, idx);
  int to_server_fd = client->to_server_fd;
  int nread = read(to_server_fd, &mesg, sizeof(mesg_t));
  check_fail(nread < 0, 1, "Error reading from client FIFO\n");
  if (mesg.kind == BL_DEPARTED) {
    log_printf("client %d '%s' DEPARTED\n", idx, client->name);
    server_remove_client(server, idx);
    server_broadcast(server, &mesg);
  }
  else if (mesg.kind == BL_MESG) {
    log_printf("client %d '%s' MESSAGE '%s'\n", idx, client->name, mesg.body);
    server_broadcast(server, &mesg);
  }
  else {
    dbg_printf("Unknown message kind received\n");
  }
  client->data_ready = 0;
  log_printf("END: server_handle_client()\n");
  return 0;
}
