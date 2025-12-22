#include "client_helper.h"
#include "client_threadpool.h"
#include <iostream>
#include <cstdlib>
#include <unistd.h>

#define DEFAULT_PORT 10000

/*
  ./client [-h host] [-p port] [-f <filename>] [-t <num_threads>]
*/
int main(int argc, char *argv[]) {
    // Default args
    std::string host = "localhost";
    int port = DEFAULT_PORT;
    std::string filename = "/";
    size_t threads = 1;

    int c;
    while ((c = getopt(argc, argv, "h:p:f:t:")) != -1) {
        switch(c) {
            case 'h':
                host = optarg;
                std::cerr << "Using host: " << host << std::endl;
                break;
            case 'p':
                port = std::atoi(optarg);
                std::cerr << "Changed port to: " << port << std::endl;
                break;
            case 'f':
                filename = optarg;
                std::cerr << "Accessing file: " << filename << std::endl;
                break;
            case 't':
                threads = std::strtoul(optarg, nullptr, 10);
                std::cerr << "Using " << threads << " threads" << std::endl;
                break;
            default:
                std::cerr << "Usage: ./client [-h host] [-p port] [-f <filename>] [-t <num_threads>]" << std::endl;
                return 1;
        }
    }

    // Create the client thread pool
    {
        Threadpool client(threads, host, port, filename);
        std::cout << "Created all client threads.\n";

        // Threadpool destructor will automatically join all threads
    } // Threadpool goes out of scope -> threads join cleanly

    std::cout << "All client threads finished.\n";
    return 0;
}
