#include "include/SocketUtils.h"
#include "include/request.h"
#include "include/WorkerPool.h"
#include "include/ThreadSafeCout.h"

using namespace std;

/*
  WebServer main entry point

  Usage: ./server [-d <basedir>] [-p <port>] [-t <num_threads>] [-b <buffer_size>]

  Options:
    -d  Root directory for serving files
    -p  Port number (default: 10000)
    -t  Number of threads in the pool
    -b  Size of the job buffer
*/

// Global thread pool pointer
ThreadPool* g_threadpool = nullptr;

int main(int argc, char* argv[]) {
    // ---- Default parameters ----
    string rootDir = ".";
    int port = DEFAULT_PORT;
    size_t numThreads = 1;
    size_t bufferSize = 3;

    // ---- Parse command-line arguments ----
    int opt;
    while ((opt = getopt(argc, argv, "d:p:t:b:")) != -1) {
        switch (opt) {
            case 'd':
                rootDir = optarg;
                ThreadSafeCout() << "[Config] Changed root directory to: " << rootDir << endl;
                break;

            case 'p':
                port = atoi(optarg);
                ThreadSafeCout() << "[Config] Changed port to: " << port << endl;
                break;

            case 't':
                numThreads = strtoul(optarg, nullptr, 10);
                ThreadSafeCout() << "[Config] Using " << numThreads << " threads" << endl;
                break;

            case 'b':
                bufferSize = strtoul(optarg, nullptr, 10);
                ThreadSafeCout() << "[Config] Using buffer size: " << bufferSize << endl;
                break;

            default:
                ThreadSafeCout() << "[Usage] ./server [-d basedir] [-p <port>] [-t <num_threads>] [-b <buffer_size>]" << endl;
                exit(1);
        }
    }

    // ---- Change working directory ----
    chdir_or_die(rootDir.c_str());

    // ---- Create listening socket ----
    int listenFd = open_listen_fd_or_die(port);

    sockaddr_in_t clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    // ---- Initialize thread pool ----
    g_threadpool = new ThreadPool(numThreads, bufferSize);

    ThreadSafeCout() << "[Server] Listening on port " << port << "..." << endl;

    // ---- Main accept loop ----
    while (true) {
        // Accept next incoming connection
        int connFd = accept_or_die(listenFd, (sockaddr_t*)&clientAddr, &clientLen);

        // Queue the job in the thread pool
        g_threadpool->queueJob(connFd);
    }

    // Cleanup (unreachable currently)
    delete g_threadpool;
    close_or_die(listenFd);

    return 0;
}
