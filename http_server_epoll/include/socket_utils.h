#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <strings.h>
#include <cassert>

#include "include/socket_utils.h"

/*
 * Create, bind, and listen on a TCP socket.
 * Returns the listening socket file descriptor.
 */
int create_listening_socket(int port) {
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    std::cerr << "[Error] Failed to create socket\n";
    return -1;
  }

  // Allow immediate reuse of the address after server restart
  int reuse_addr = 1;
  if (setsockopt(
        listen_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &reuse_addr,
        sizeof(reuse_addr)) < 0) {
    std::cerr << "[Error] setsockopt(SO_REUSEADDR) failed\n";
    return -1;
  }

  // Initialize server address structure
  sockaddr_in_t server_addr;
  bzero(&server_addr, sizeof(server_addr));

  server_addr.sin_family      = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port        = htons(static_cast<unsigned short>(port));

  // Bind socket to address and port
  if (bind(
        listen_fd,
        reinterpret_cast<sockaddr_t*>(&server_addr),
        sizeof(server_addr)) < 0) {
    std::cerr << "[Error] bind() failed\n";
    return -1;
  }

  // Start listening for incoming connections
  if (listen(listen_fd, LISTEN_BACKLOG) < 0) {
    std::cerr << "[Error] listen() failed\n";
    return -1;
  }

  return listen_fd;
}
