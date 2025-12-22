#include "include/SocketUtils.h"

// for htons, htonl
#include <arpa/inet.h>  

// for memset
#include <cstring>      

// Set up a socket to listen for incoming connections
int open_listen_fd(int port) {

    // Create socket
    int listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        std::cerr << "socket failed" << std::endl;
        return -1;
    }

    // Eliminates "Address already in use" error from bind
    int optval = 1;
    if (::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        std::cerr << "setsockopt failed" << std::endl;
        return -1;
    }

    // Prepare server address
    sockaddr_in_t server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // **no :: here**
    server_addr.sin_port = htons(static_cast<uint16_t>(port)); // **no :: here**

    // Bind the socket
    if (::bind(listen_fd, reinterpret_cast<sockaddr_t*>(&server_addr), sizeof(server_addr)) < 0) {
        std::cerr << "bind failed" << std::endl;
        return -1;
    }

    // Listen for incoming connections
    if (::listen(listen_fd, QUEUE_SIZE) < 0) {
        std::cerr << "listen failed" << std::endl;
        return -1;
    }

    return listen_fd;
}
