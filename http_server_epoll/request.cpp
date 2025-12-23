#include <sstream>
#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <sys/stat.h>

#include "include/request.h"

/*
 * Determine MIME type based on file extension
 */
static std::string get_mime_type(const std::string& filename) {
  if (filename.find(".html") != std::string::npos) return "text/html";
  if (filename.find(".gif")  != std::string::npos) return "image/gif";
  if (filename.find(".jpg")  != std::string::npos) return "image/jpeg";
  return "text/plain";
}

/*
 * Serve a static file to the client
 */
static void serve_static_file(int client_fd,
                              const std::string& filepath,
                              size_t file_size) {
  std::string mime_type = get_mime_type(filepath);

  int file_fd = OPEN_OR_DIE(filepath.c_str(), O_RDONLY, 0);
  void* file_data = MMAP_OR_DIE(nullptr, file_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
  CLOSE_OR_DIE(file_fd);

  std::ostringstream header;
  header << "HTTP/1.0 200 OK\r\n"
         << "Server: WebServer\r\n"
         << "Content-Length: " << file_size << "\r\n"
         << "Content-Type: " << mime_type << "\r\n\r\n";

  std::string header_str = header.str();
  SEND_OR_DIE(client_fd, header_str.c_str(), header_str.size(), 0);
  SEND_OR_DIE(client_fd, file_data, file_size, 0);

  MUNMAP_OR_DIE(file_data, file_size);
}

/*
 * Serve a CGI (dynamic) request
 */
static void serve_dynamic_content(int client_fd,
                                  const std::string& executable,
                                  const std::string& cgi_args) {
  const char response[] = "HTTP/1.0 200 OK\r\nServer: WebServer\r\n";
  SEND_OR_DIE(client_fd, response, strlen(response), 0);

  char* argv[] = { nullptr };

  if (FORK_OR_DIE() == 0) {
    SETENV_OR_DIE("QUERY_STRING", cgi_args.c_str(), 1);
    DUP2_OR_DIE(client_fd, STDOUT_FILENO);

    extern char** environ;
    EXECVE_OR_DIE(executable.c_str(), argv, environ);
  } else {
    WAIT_OR_DIE(nullptr);
  }
}

/*
 * Send an HTTP error response
 */
static void send_error_response(int client_fd,
                                const std::string& status_code,
                                const std::string& short_msg,
                                const std::string& long_msg,
                                const std::string& cause) {
  std::ostringstream body;
  body << "<!doctype html>\r\n"
       << "<head><title>WebServer Error</title></head>\r\n"
       << "<body>\r\n"
       << "<h2>" << status_code << ": " << short_msg << "</h2>\r\n"
       << "<p>" << long_msg << ": " << cause << "</p>\r\n"
       << "</body>\r\n</html>\r\n";

  std::string body_str = body.str();

  std::ostringstream header;
  header << "HTTP/1.0 " << status_code << " " << short_msg << "\r\n"
         << "Content-Type: text/html\r\n"
         << "Content-Length: " << body_str.size() << "\r\n\r\n";

  std::string header_str = header.str();
  SEND_OR_DIE(client_fd, header_str.c_str(), header_str.size(), 0);
  SEND_OR_DIE(client_fd, body_str.c_str(), body_str.size(), 0);
}

/*
 * Parse URI and determine whether request is static or dynamic
 */
static bool parse_request_uri(const std::string& uri,
                              std::string& resolved_path,
                              std::string& cgi_args) {
  if (uri.find("cgi") == std::string::npos) {
    cgi_args.clear();
    resolved_path = "." + uri;
    if (uri.back() == '/') {
      resolved_path += "index.html";
    }
    return true;
  }

  size_t query_pos = uri.find('?');
  cgi_args = (query_pos == std::string::npos) ? "" : uri.substr(query_pos + 1);
  resolved_path = "." + uri;
  return false;
}

/*
 * Main request handler
 */
void handle_http_request(int client_fd) {
  char buffer[REQUEST_BUFFER_SIZE];
  ssize_t bytes_read = recv(client_fd, buffer, REQUEST_BUFFER_SIZE - 1, 0);

  if (bytes_read <= 0) {
    std::cerr << "[Error] recv() failed\n";
    return;
  }

  buffer[bytes_read] = '\0';
  std::istringstream request_stream(buffer);

  std::string method, uri, version;
  request_stream >> method >> uri >> version;

  std::cout << "[Request] " << method << " " << uri << " " << version << std::endl;

  if (method != "GET") {
    send_error_response(client_fd,
                        "501",
                        "Not Implemented",
                        "Only GET method is supported",
                        method);
    return;
  }

  std::string filepath, cgi_args;
  bool is_static = parse_request_uri(uri, filepath, cgi_args);

  struct stat file_stat;
  if (stat(filepath.c_str(), &file_stat) < 0) {
    send_error_response(client_fd,
                        "404",
                        "Not Found",
                        "File not found",
                        filepath);
    return;
  }

  if (is_static) {
    if (!S_ISREG(file_stat.st_mode) || !(S_IRUSR & file_stat.st_mode)) {
      send_error_response(client_fd,
                          "403",
                          "Forbidden",
                          "Access denied",
                          filepath);
      return;
    }
    serve_static_file(client_fd, filepath, file_stat.st_size);
  } else {
    if (!S_ISREG(file_stat.st_mode) || !(S_IXUSR & file_stat.st_mode)) {
      send_error_response(client_fd,
                          "403",
                          "Forbidden",
                          "CGI execution denied",
                          filepath);
      return;
    }
    serve_dynamic_content(client_fd, filepath, cgi_args);
  }
}
