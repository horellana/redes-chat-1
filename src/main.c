#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define error(msg) do { \
      int buffer_length = 2048; \
      char buffer[buffer_length]; \
      snprintf(buffer, buffer_length, "Error: %s\n: Linea: %d\n", msg, __LINE__); \
      perror(buffer);                                                   \
  } while (0)


struct Client {
  int socket;
  int connected;
};

struct Server {
  int socket;
  int port;

  struct Client clients[1024];
  int client_count;

  char messages[1024][4096];
};

int create_server(int port, struct Server *server) {
  int server_socket;

  struct sockaddr_in serv_addr, cli_addr;

  server_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (server_socket < 0) {
    return server_socket;
  }

  /* Define el maximo de tiempo que un socket bloquea al leer un mensaje */
  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  int r = setsockopt(server_socket,
                     SOL_SOCKET,
                     SO_RCVTIMEO,
                     (char *)&timeout,
                     sizeof(timeout));

  if (r < 0) {
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  int bind_result = bind(server_socket,
                         (struct sockaddr *)&serv_addr,
                         sizeof(serv_addr));

  if (bind_result < 0) {
    return bind_result;
  }

  if (listen(server_socket, 5) < 0) {
    return -1;
  }

  server->socket = server_socket;
  server->port = port;
  server->client_count = 0;

  return 0;
}

int accept_client(struct Server *server) {
  struct sockaddr_in client_addr;
  int client_size = sizeof(client_addr);

  int client_socket = accept(server->socket,
                             (struct sockaddr *)&client_addr,
                             &client_size);

  if (client_socket < 0 ) {
    return errno == EWOULDBLOCK ? 0 : -1;
  }

  struct Client client = { .socket = client_socket, .connected = true };

  server->clients[server->client_count] = client;
  server->client_count++;

  return 0;
}

void broadcast(struct Server *server, char *message, int message_length) {
  for (int i = 0; i < server->client_count; i++) {
    for (int fails = 0; fails < 5; fails++) {
      int r = send(server->clients[i].socket, message, message_length, 0);

      if (r == 0) {
        break;
      }
    }
  }
}

int accept_message(struct Server *server) {
  for (int i = 0; i < server->client_count; i++) {
    int buffer_length = 4096;
    char buffer[buffer_length];
    memset(buffer, '\0', buffer_length);

    int r = recv(server->clients[i].socket, buffer, buffer_length, 0);

    if (r < 0 && errno == EWOULDBLOCK) {
      continue;
    }

    for (int j = 0; j < server->client_count; j++) {
      if (j == i) {
        continue;
      }

      broadcast(server, buffer, r);
    }
  }

  return 0;
}

int main(int argc, char **argv) {
  int port;

  if (argc < 2) {
    port = 9999;
  }
  else {
    port = atoi(argv[1]);
  }

  struct Server server;

  if (create_server(port, &server) < 0) {
    error("Error al iniciar el servidor: ");
    return -1;
  }

  fprintf(stderr, "DEBUG: Servidor iniciado y escuchando en el puerto: %d\n", server.port);

  while (true) {
    int client_socket = accept_client(&server);

    if (client_socket < 0) {
      error("Error al aceptar un nuevo cliente");
    }
  }

  return 0;
}
