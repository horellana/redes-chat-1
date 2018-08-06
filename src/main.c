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

struct Server {
  int socket;
  int port;
  int clients[1024];
  int client_count;
};

int create_server(int port, struct Server *server) {
  int server_socket;

  struct sockaddr_in serv_addr, cli_addr;

  server_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (server_socket < 0) {
    return server_socket;
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

  if (client_socket < 0) {
    return -1;
  }

  server->clients[server->client_count] = client_socket;
  server->client_count++;

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

  printf("Servidor iniciado y escuchando en el puerto: %d\n", server.port);

  int client_socket = accept_client(&server);

    if (client_socket < 0) {
      error("Error al aceptar un nuevo cliente");
      return -1;
    }
  }

  printf("Nuevo client\n Numero de clientes conectados: %d\n", server.client_count);

  return 0;
}
