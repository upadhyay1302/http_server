#include "client_threadpool.h"
#include "client_helper.h"
#include <iostream>
#include <thread>

using namespace std;


Threadpool::Threadpool(size_t num_threads, const string &server_addr, int server_port, const string &file)
    : port(server_port), hostname(server_addr), filename(file), running(true)
{
    for(size_t i = 0; i < num_threads; ++i){
        threads.emplace_back(&Threadpool::sendRequest, this);
    }
}

Threadpool::~Threadpool() {
    // Stop the threads
    running = false;

    // Join all threads
    for(auto &t : threads){
        if(t.joinable())
            t.join();
    }
}

void Threadpool::sendRequest() {
    while(running) {
        // Slight delay to prevent rapid socket exhaustion
        this_thread::sleep_for(chrono::milliseconds(10));

        // Open connection to the server
        int clientfd = open_client_fd_or_die(hostname.c_str(), port);

        // Send request
        client_send(clientfd, filename);

        // Receive response
        client_recv(clientfd);

        // Close connection
        close_or_die(clientfd);
    }
}
