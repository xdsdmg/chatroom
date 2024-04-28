#include <libssh/libssh.h>
#include <libssh/server.h>
#include <stdio.h>
#include <stdlib.h>

#define USERNAME "user"
#define PASSWORD "password"

int authenticate_password(ssh_session session, const char *username,
                          const char *password) {
  int auth_status = ssh_userauth_password(session, username, password);
  if (auth_status != SSH_AUTH_SUCCESS) {
    fprintf(stderr, "Authentication failed: %s\n", ssh_get_error(session));
    return -1;
  }
  return 0;
}

int main() {
  ssh_bind sshbind;
  ssh_session session;
  ssh_message message;

  // Create SSH bind object
  sshbind = ssh_bind_new();
  if (sshbind == NULL) {
    fprintf(stderr, "Failed to create SSH bind object\n");
    return 1;
  }

  // Set options
  ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDADDR, "0.0.0.0");
  ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_BINDPORT_STR, "6789");

  // Start listening for connections
  if (ssh_bind_listen(sshbind) < 0) {
    fprintf(stderr, "Failed to start listening for connections: %s\n",
            ssh_get_error(sshbind));
    ssh_bind_free(sshbind);
    return 1;
  }

  printf("Listening for connections on port 2222...\n");

  // Accept connections
  session = ssh_new();
  if (session == NULL) {
    fprintf(stderr, "Failed to create SSH session\n");
    ssh_bind_free(sshbind);
    return 1;
  }

  while (ssh_bind_accept(sshbind, session) == SSH_OK) {
    printf("Accepted connection\n");

    // Perform authentication
    if (authenticate_password(session, USERNAME, PASSWORD) != 0) {
      ssh_disconnect(session);
      continue;
    }

    printf("Authentication succeeded\n");

    // Handle SSH commands here

    // Disconnect session
    ssh_disconnect(session);
  }

  // Free resources
  ssh_bind_free(sshbind);
  ssh_free(session);

  return 0;
}

