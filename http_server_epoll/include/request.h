#pragma once

#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <cassert>

/*
 * Request handling constants
 */
constexpr size_t REQUEST_BUFFER_SIZE = 8192;

/*
 * Entry point for handling a single HTTP request
 */
void handle_http_request(int client_fd);

/*
 * System-call wrappers that abort on failure
 */
#define OPEN_OR_DIE(path, flags, mode) \
  ({ int fd = open(path, flags, mode); assert(fd >= 0); fd; })

#define CLOSE_OR_DIE(fd) \
  assert(close(fd) == 0)

#define SEND_OR_DIE(fd, buffer, length, flags) \
  ({ ssize_t rc = send(fd, buffer, length, flags); assert(rc >= 0); rc; })

#define MMAP_OR_DIE(addr, len, prot, flags, fd, offset) \
  ({ void *ptr = mmap(addr, len, prot, flags, fd, offset); assert(ptr != MAP_FAILED); ptr; })

#define MUNMAP_OR_DIE(addr, len) \
  assert(munmap(addr, len) == 0)

#define FORK_OR_DIE() \
  ({ pid_t pid = fork(); assert(pid >= 0); pid; })

#define SETENV_OR_DIE(name, value, overwrite) \
  assert(setenv(name, value, overwrite) == 0)

#define DUP2_OR_DIE(src, dst) \
  assert(dup2(src, dst) >= 0)

#define EXECVE_OR_DIE(path, argv, envp) \
  assert(execve(path, argv, envp) == 0)

#define WAIT_OR_DIE(status) \
  ({ pid_t pid = wait(status); assert(pid >= 0); pid; })
