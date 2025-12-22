#include "client_helper.h"
#include <cstring>   // For memset, memcpy

int open_client_fd(const char *hostname, int port) {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) return -1;

    struct hostent *hp = gethostbyname(hostname);
    if (!hp) return -2;

    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    std::memcpy(&server_addr.sin_addr.s_addr, hp->h_addr, hp->h_length);
    server_addr.sin_port = htons(static_cast<uint16_t>(port));

    if (connect(client_fd, reinterpret_cast<sockaddr_t*>(&server_addr), sizeof(server_addr)) < 0)
        return -1;

    return client_fd;
}

void client_send(int fd, const std::string &filename){
    std::array<char, MAXBUF> hostname{};
    gethostname_or_die(hostname.data(), hostname.size());

    std::ostringstream request;
    request << "GET " << filename << " HTTP/1.1\r\n";
    request << "Host: " << hostname.data() << "\r\n\r\n";

    std::string request_str = request.str();
    send_or_die(fd, request_str.c_str(), request_str.size(), 0);
}

void client_recv(int fd){
    std::array<char, MAXBUF> buf{};
    ssize_t bytes_received;

    // First recv() call for headers
    if ((bytes_received = recv(fd, buf.data(), buf.size() - 1, 0)) < 0){
        std::cerr << "recv1 failed" << std::endl;
        return;
    }
    buf[bytes_received] = '\0';
    std::string response(buf.data());

    // Second recv() call for body
    if ((bytes_received = recv(fd, buf.data(), buf.size() - 1, 0)) < 0){
        std::cerr << "recv2 failed" << std::endl;
        return;
    }
    buf[bytes_received] = '\0';
    response = buf.data();
}
