#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#define BUFFER_SIZE 1024

#define error(msg) do { \
      char buffer[BUFFER_SIZE]; \
      snprintf(buffer, BUFFER_SIZE, "Error: %s\n: Linea: %d\n", msg, __LINE__); \
      /* perror(buffer);        */                                           \
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
};

int configure_async_socket(int socket) {
#ifdef _WIN32
	int iMode = 1;
	int iResult = ioctlsocket(socket, FIONBIO, &iMode);
	if (iResult != NO_ERROR) {
		return -1;
	}
#endif

	return 0;
}

int config_server(int server) {
	int enable = 1;
	int r = setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	if (r < 0) {
		return -1;
	}

	r = configure_async_socket(server);

	if (r < 0) {
		return -1;
	}

	return 0;
}

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

  if (config_server(server_socket) < 0) {
	  return -1;
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

void send_message(int client, char *message, int message_length, int sender) {
	if (message_length < 0)
		return;

	char buffer[BUFFER_SIZE];
	int buffer_length = message_length + 3;

	memset(buffer, '\0', BUFFER_SIZE);
	snprintf(buffer, buffer_length, "%d: %s", sender, message);

	send(client, buffer, buffer_length, 0);
}

void broadcast(struct Server *server, char *message, int message_length, int sender) {
  for (int i = 0; i < server->client_count; i++) {
	  if (i != sender) {
		  send_message(server->clients[i].socket, message, message_length, sender);
	  }
  }
}

int accept_message(struct Server *server) {
  for (int i = 0; i < server->client_count; i++) {
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', BUFFER_SIZE);

    int r = recv(server->clients[i].socket, buffer, BUFFER_SIZE, 0);

    if (r < 0 && errno == EWOULDBLOCK) {
      continue;
    }
	else {
		broadcast(server, buffer, r, i);
	}
	
  }

  return 0;
}

int main(int argc, char **argv) {
  	#ifdef _WIN32
	WSADATA wsaData;

	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		fprintf(stderr, "WSAStartup failed: %d\n", iResult);
		return 1;
	}
	#endif
      
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
	else {
		puts("Accepted a new client");
	}

	accept_message(&server);

	Sleep(100);
  }

  return 0;
}
