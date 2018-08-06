#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int create_server(int port) {
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

  return server_socket;
}

int main(int argc, char **argv) {
  int port;

  if (argc < 2) {
    port = 9999;
  }
  else {
    port = atoi(argv[1]);
  }

  int server_socket = create_server(port);

  if (server_socket < 0) {
    perror("Error al iniciar el servidor: ");
    return -1;
  }

  printf("Servidor iniciado\n", server_socket);

  return 0;
}
