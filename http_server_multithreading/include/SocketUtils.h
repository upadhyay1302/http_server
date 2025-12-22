#pragma once

#include <iostream>
#include <unistd.h>
#include <cassert>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netdb.h>
#include <fcntl.h>
#include <cstring>
#include <arpa/inet.h>

using namespace std;

// Socket type aliases
using sockaddr_t    = struct sockaddr;
using sockaddr_in_t = struct sockaddr_in;

// Default server configuration
constexpr int DEFAULT_PORT = 10000;
constexpr int QUEUE_SIZE   = 1024;

// Open a listening socket on the given port
int open_listen_fd(int port);

// ----- Convenience wrappers (error-checked) -----
inline int open_listen_fd_or_die(int port) {
    int fd = open_listen_fd(port);
    assert(fd >= 0);
    return fd;
}

inline void chdir_or_die(const char* path) {
    assert(chdir(path) == 0);
}

inline int accept_or_die(int s, sockaddr_t* addr, socklen_t* addrlen) {
    int conn_fd = accept(s, addr, addrlen);
    assert(conn_fd >= 0);
    return conn_fd;
}

inline int open_or_die(const char* pathname, int flags, int mode) {
    int fd = open(pathname, flags, mode);
    assert(fd >= 0);
    return fd;
}

inline void close_or_die(int fd) {
    assert(close(fd) == 0);
}

inline ssize_t send_or_die(int fd, const void* buf, size_t count, int flags) {
    ssize_t rc = send(fd, buf, count, flags);
    assert(rc >= 0);
    return rc;
}

inline void* mmap_or_die(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
    void* ptr = mmap(addr, length, prot, flags, fd, offset);
    assert(ptr != MAP_FAILED);
    return ptr;
}

inline void munmap_or_die(void* addr, size_t length) {
    assert(munmap(addr, length) == 0);
}

inline pid_t fork_or_die() {
    pid_t pid = fork();
    assert(pid >= 0);
    return pid;
}

inline int setenv_or_die(const char* name, const char* value, int overwrite) {
    int rc = setenv(name, value, overwrite);
    assert(rc == 0);
    return rc;
}

inline int dup2_or_die(int fd1, int fd2) {
    int rc = dup2(fd1, fd2);
    assert(rc >= 0);
    return rc;
}

inline void execve_or_die(const char* filename, char* const argv[], char* const envp[]) {
    assert(execve(filename, argv, envp) == 0);
}

inline pid_t wait_or_die(int* status) {
    pid_t pid = wait(status);
    assert(pid >= 0);
    return pid;
}
