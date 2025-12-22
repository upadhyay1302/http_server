#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <array>
#include <cassert>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct sockaddr sockaddr_t;

constexpr size_t MAXBUF = 16384;

/* Establish connection w/ server */
int open_client_fd(const char *hostname, int port);

/* Send request to server */
void client_send(int fd, const std::string &filename);

/* Receive HTTP response from server */
void client_recv(int fd);

/* Helper macros for convenience */
#define open_client_fd_or_die(hostname, port) \
    ({ int rc = open_client_fd(hostname, port); assert(rc >= 0); rc; })

#define gethostname_or_die(name, len) \
    ({ int rc = gethostname(name, len); assert(rc == 0); rc; })

#define send_or_die(fd, buf, count, flags) \
    ({ ssize_t rc = send(fd, buf, count, flags); assert(rc >= 0); rc; })

#define close_or_die(fd) \
    assert(close(fd) == 0); 
