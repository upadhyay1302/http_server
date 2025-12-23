#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <strings.h>
#include <assert.h>
#include "include/socket_utils.h"

// Set up a socket to listen for incoming connections
int open_listen_fd(int port){

  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    std::cerr << "socket fail" << std::endl;
    return -1;
  }

  //For use in non-blocking IO setup
  // if(fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1){
  //   std::cerr << "fcntl fail" << std::endl;
  //   return -1;
  // }
  // Additionally make every new socket non-blocking

  // Eliminates "Address already in use" error from bind
  int optval = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int)) < 0) {
    std::cerr << "setsockopt fail" << std::endl;
    return -1;
  }

  // Setting up a socket struct shenanigans
  sockaddr_in_t server_addr;
  bzero((char *) &server_addr, sizeof(server_addr)); // Zero memory
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
  server_addr.sin_port = htons((unsigned short) port);
  // server_addr is loaded up with IP address and port to bind to

  if (bind(sockfd, (sockaddr_t *) &server_addr, sizeof(server_addr)) < 0){
    std::cerr << "bind failed" << std::endl;
    return -1;
  }

  // Listen for incoming connections
  if (listen(sockfd, QUEUE_SIZE) < 0) {
    std::cerr << "listen failed" << std::endl;
    return -1;
  }

  return sockfd;
}