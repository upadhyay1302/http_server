#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <cstdlib>

#include "include/socket_utils.h"
#include "include/request.h"

/*
 * Usage:
 *   ./server [-d <basedir>] [-p <port>]
 */
int main(int argc, char* argv[]) {

  /* ----------------------------
   * Parse command-line arguments
   * ---------------------------- */
  std::string base_directory = ".";
  int port = DEFAULT_PORT;

  int option;
  while ((option = getopt(argc, argv, "d:p:")) != -1) {
    switch (option) {
      case 'd':
        base_directory = optarg;
        std::cerr << "[Config] Base directory set to " << base_directory << std::endl;
        break;

      case 'p':
        port = std::atoi(optarg);
        std::cerr << "[Config] Port set to " << port << std::endl;
        break;

      default:
        std::cerr << "Usage: ./server [-d basedir] [-p port]\n";
        std::exit(1);
    }
  }

  chdir_or_die(base_directory.c_str());

  /* ----------------------------
   * Create listening socket
   * ---------------------------- */
  int listen_fd = open_listen_fd_or_die(port);
  std::cout << "[Server] Listening on port " << port << std::endl;

  /* ----------------------------
   * Initialize epoll
   * ---------------------------- */
  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    std::cerr << "[Error] epoll_create1 failed\n";
    std::exit(1);
  }

  struct epoll_event event {};
  event.data.fd = listen_fd;
  event.events = EPOLLIN;

  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) == -1) {
    std::cerr << "[Error] epoll_ctl ADD listen_fd failed\n";
    std::exit(1);
  }

  /* ----------------------------
   * Event loop
   * ---------------------------- */
  struct epoll_event ready_events[MAX_EVENTS];

  std::cout << "[Server] Entering event loop\n";

  while (true) {
    int num_ready = epoll_wait(epoll_fd, ready_events, MAX_EVENTS, -1);
    if (num_ready == -1) {
      std::cerr << "[Error] epoll_wait failed\n";
      std::exit(1);
    }

    for (int i = 0; i < num_ready; ++i) {
      int fd = ready_events[i].data.fd;

      if (ready_events[i].events & EPOLLIN) {

        /* ----------------------------
         * New incoming connection
         * ---------------------------- */
        if (fd == listen_fd) {
          int client_fd = accept_or_die(listen_fd);
          std::cout << "[Server] Accepted new connection (fd=" << client_fd << ")\n";

          struct epoll_event client_event {};
          client_event.data.fd = client_fd;
          client_event.events = EPOLLIN;

          if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
            std::cerr << "[Error] epoll_ctl ADD client_fd failed\n";
            close(client_fd);
          }
        }
        /* ----------------------------
         * Existing client request
         * ---------------------------- */
        else {
          int client_fd = fd;
          std::cout << "[Server] Handling request (fd=" << client_fd << ")\n";

          handle_http_request(client_fd);
          close(client_fd);
        }
      }
    }
  }

  return 0;
}
