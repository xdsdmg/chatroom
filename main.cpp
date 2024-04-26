#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct sockaddr SA;

#define PORT 6789

int Socket(int __domain, int __type, int __protocol) {
  int listen_socket;

  if ((listen_socket = socket(__domain, __type, __protocol)) < 0) {
    fprintf(stderr, "failed to create socket: %s\n", strerror(errno));
    exit(1);
  }

  return listen_socket;
}

void Bind(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len) {
  if (bind(__fd, __addr, __len) < 0) {
    fprintf(stderr, "failed to bind socket: %s\n", strerror(errno));
    close(__fd);
    exit(1);
  }
}

void Listen(int __fd, int __n) {
  if (listen(__fd, __n) < 0) {
    fprintf(stderr, "failed to listen on socket: %s\n", strerror(errno));
    close(__fd);
    exit(1);
  }
}

int Accept(int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len) {
  int conn_socket;
  if ((conn_socket = accept(__fd, __addr, __addr_len)) < 0) {
    fprintf(stderr, "failed to accept client connection: %s\n",
            strerror(errno));
    close(__fd);
    exit(1);
  }

  return conn_socket;
}

int main(int argc, char **argv) {
  int listen_socket = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  Bind(listen_socket, (SA *)&addr, sizeof(addr));
  Listen(listen_socket, SOMAXCONN);

  printf("listening on port %d\n", PORT);

  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  int client_socket =
      Accept(listen_socket, (SA *)&client_addr, &client_addr_len);

  printf("[INFO] client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr),
         ntohs(client_addr.sin_port));

  close(client_socket);
  close(listen_socket);

  return 0;
}
