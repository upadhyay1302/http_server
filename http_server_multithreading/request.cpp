#include <sstream>
#include <cstring>
#include <sys/stat.h>
#include "include/SocketUtils.h"
#include "include/ThreadSafeCout.h"
#include "include/request.h"
#include "include/WorkerPool.h"

using namespace std;

// Global thread pool (extern)
extern ThreadPool* g_threadpool;

// ---- Helper: Send HTTP error ----
static void sendError(int fd, const string& errCode, const string& shortMsg,
                      const string& longMsg, const string& cause) {
    ostringstream body, response;

    body << "<!doctype html>\r\n"
         << "<head>\r\n"
         << "  <title>WebServer Error</title>\r\n"
         << "</head>\r\n"
         << "<body>\r\n"
         << "  <h2>" << errCode << ": " << shortMsg << "</h2>\r\n"
         << "  <p>" << longMsg << ": " << cause << "</p>\r\n"
         << "</body>\r\n"
         << "</html>\r\n";

    string bodyStr = body.str();

    response << "HTTP/1.0 " << errCode << " " << shortMsg << "\r\n"
             << "Content-Type: text/html\r\n"
             << "Content-Length: " << bodyStr.length() << "\r\n\r\n";

    string headerStr = response.str();

    send_or_die(fd, headerStr.c_str(), headerStr.length(), 0);
    send_or_die(fd, bodyStr.c_str(), bodyStr.length(), 0);

    ThreadSafeCout() << "[Request FD=" << fd << "] Sent HTTP error: " << errCode 
                 << " " << shortMsg << " (Cause: " << cause << ")" << endl;
}

// ---- Helper: Determine content type ----
static void getFileType(const string& filename, string& fileType) {
    if (filename.find(".html") != string::npos)
        fileType = "text/html";
    else if (filename.find(".gif") != string::npos)
        fileType = "image/gif";
    else if (filename.find(".jpg") != string::npos)
        fileType = "image/jpeg";
    else
        fileType = "text/plain";
}

// ---- Serve static files ----
static void serveStatic(int fd, const string& filename, int fileSize) {
    ThreadSafeCout() << "[Request FD=" << fd << "] Serving static file: " << filename
                 << " (" << fileSize << " bytes)" << endl;

    string fileType;
    getFileType(filename, fileType);

    int srcFd = open_or_die(filename.c_str(), O_RDONLY, 0);
    void* srcPtr = mmap_or_die(0, fileSize, PROT_READ, MAP_PRIVATE, srcFd, 0);
    close_or_die(srcFd);

    ostringstream response;
    response << "HTTP/1.0 200 OK\r\n"
             << "Server: WebServer\r\n"
             << "Content-Length: " << fileSize << "\r\n"
             << "Content-Type: " << fileType << "\r\n\r\n";

    string headerStr = response.str();

    send_or_die(fd, headerStr.c_str(), headerStr.length(), 0);
    send_or_die(fd, static_cast<char*>(srcPtr), fileSize, 0);
    munmap_or_die(srcPtr, fileSize);

    ThreadSafeCout() << "[Request FD=" << fd << "] Finished serving: " << filename << endl;
}

// ---- Serve dynamic CGI ----
static void serveDynamic(int fd, const string& filename, const string& cgiArgs) {
    ThreadSafeCout() << "[Request FD=" << fd << "] Running CGI: " << filename 
                 << " Args: '" << cgiArgs << "'" << endl;

    struct stat sbuf;
    if (stat(filename.c_str(), &sbuf) < 0) {
        sendError(fd, "404", "Not Found", "CGI program not found", filename);
        return;
    }

    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
        sendError(fd, "403", "Forbidden", "CGI program not executable", filename);
        return;
    }

    char buf[MAXBUF] = "HTTP/1.0 200 OK\r\nServer: WebServer\r\n";
    send_or_die(fd, buf, strlen(buf), 0);

    char* argv[] = { nullptr };
    pid_t pid = fork();

    if (pid < 0) {
        sendError(fd, "500", "Internal Server Error", "Failed to fork", filename);
        return;
    } 
    else if (pid == 0) {
        setenv_or_die("QUERY_STRING", cgiArgs.c_str(), 1);
        dup2_or_die(fd, STDOUT_FILENO);
        extern char **environ;
        execve(filename.c_str(), argv, environ);
        exit(1); // execve failed
    } 
    else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
            ThreadSafeCout() << "[Request FD=" << fd << "] CGI executed successfully" << endl;
        else
            ThreadSafeCout() << "[Request FD=" << fd << "] CGI exited with error: " 
                         << WEXITSTATUS(status) << endl;
    }
}

// ---- Parse URI ----
static bool parseUri(const string& uri, string& filename, string& cgiArgs) {
    if (uri.find("cgi") == string::npos) {
        // Static content
        cgiArgs = "";
        filename = "." + uri;
        if (uri.back() == '/')
            filename += "index.html";
        ThreadSafeCout() << "[Request] Parsed URI '" << uri << "' as static: " << filename << endl;
        return true;
    } else {
        // Dynamic content
        size_t qmark = uri.find("?");
        cgiArgs = (qmark == string::npos) ? "" : uri.substr(qmark + 1);
        filename = "." + uri;
        ThreadSafeCout() << "[Request] Parsed URI '" << uri << "' as dynamic: " << filename
                     << " Args: '" << cgiArgs << "'" << endl;
        return false;
    }
}

// ---- Serve /metrics ----
static void serveMetrics(int fd) {
    ostringstream body;
    body << "active_threads " << g_threadpool->getActiveThreads() << "\n"
         << "live_threads "   << g_threadpool->getLiveThreads()   << "\n"
         << "queue_size "     << g_threadpool->getQueueSize()     << "\n"
         << "total_requests " << g_threadpool->getTotalRequests() << "\n";

    string bodyStr = body.str();
    ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/plain\r\n"
             << "Content-Length: " << bodyStr.size() << "\r\n\r\n";

    string header = response.str();
    send_or_die(fd, header.c_str(), header.size(), 0);
    send_or_die(fd, bodyStr.c_str(), bodyStr.size(), 0);
}

// ---- Handle incoming request ----
void handle_request(int fd) {
    char buf[MAXBUF];
    ssize_t bytes = recv(fd, buf, MAXBUF - 1, 0);
    if (bytes <= 0) {
        ThreadSafeCout() << "[Request FD=" << fd << "] Failed to receive data" << endl;
        return;
    }
    buf[bytes] = '\0';

    istringstream reqStream(buf);
    string method, uri, version;
    reqStream >> method >> uri >> version;

    ThreadSafeCout() << "[Request FD=" << fd << "] Received request: Method=" << method
                 << " URI=" << uri << " Version=" << version << endl;

    if (method != "GET") {
        sendError(fd, "501", "Not Implemented", "HTTP method not supported", method);
        return;
    }

    // Handle /metrics endpoint
    if (uri == "/metrics") {
        serveMetrics(fd);
        close_or_die(fd);
        ThreadSafeCout() << "[Request FD=" << fd << "] Served /metrics" << endl;
        return;
    }

    // --- Handle static or dynamic file ---
    string filename, cgiArgs;
    bool isStatic = parseUri(uri, filename, cgiArgs);

    struct stat sbuf;
    if (stat(filename.c_str(), &sbuf) < 0) {
        sendError(fd, "404", "Not Found", "File not found", filename);
        return;
    }

    if (isStatic) {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            sendError(fd, "403", "Forbidden", "Cannot read file", filename);
            return;
        }
        serveStatic(fd, filename, sbuf.st_size);
    } else {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            sendError(fd, "403", "Forbidden", "Cannot execute CGI", filename);
            return;
        }
        serveDynamic(fd, filename, cgiArgs);
    }
}
