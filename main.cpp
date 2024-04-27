#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <libssh2.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct sockaddr SA;

#define PORT 6789
#define USERNAME "user"
#define PASSWORD "password"

int Socket(int domain, int type, int protocol) {
  int listen_socket;

  if ((listen_socket = socket(domain, type, protocol)) < 0) {
    fprintf(stderr, "failed to create socket: %s\n", strerror(errno));
    exit(1);
  }

  return listen_socket;
}

void Bind(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len) {
  if (bind(fd, addr, len) < 0) {
    fprintf(stderr, "failed to bind socket: %s\n", strerror(errno));
    close(fd);
    exit(1);
  }
}

void Listen(int fd, int n) {
  if (listen(fd, n) < 0) {
    fprintf(stderr, "failed to listen on socket: %s\n", strerror(errno));
    close(fd);
    exit(1);
  }
}

int Accept(int fd, __SOCKADDR_ARG addr, socklen_t *__restrict addr_len) {
  int conn_socket;

  if ((conn_socket = accept(fd, addr, addr_len)) < 0) {
    fprintf(stderr, "failed to accept client connection: %s\n",
            strerror(errno));
    close(fd);
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

  while (true) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int conn_socket =
        Accept(listen_socket, (SA *)&client_addr, &client_addr_len);
    printf("[INFO] client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    LIBSSH2_SESSION *session = libssh2_session_init();
    if (session == NULL) {
      printf("failed to create libssh2 session\n");
      close(conn_socket);
      continue;
    }

    printf("a\n");

    // Set up the TCP/IP socket for the session
    if (libssh2_session_handshake(session, conn_socket) != 0) {
      printf("failed to establish SSH session\n");
      libssh2_session_free(session);
      close(conn_socket);
      continue;
    }

    printf("b\n");

    // Perform user authentication
    if (libssh2_userauth_password(session, USERNAME, PASSWORD) != 0) {
      printf("authentication failed\n");
      libssh2_session_disconnect(session, "authentication failed");
      libssh2_session_free(session);
      close(conn_socket);
      continue;
    }

    printf("authentication succeeded\n");

    // Handle SSH commands from the client

    // Clean up
    libssh2_session_disconnect(session, "server shutting down");
    libssh2_session_free(session);

    close(conn_socket);
  }

  close(listen_socket);

  return 0;
}
