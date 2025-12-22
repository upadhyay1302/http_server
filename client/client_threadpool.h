#pragma once

#include <vector>
#include <thread>
#include <string>
#include <atomic>
#include <chrono>

using namespace std;

class Threadpool {
private:
    int port;
    string hostname;
    string filename;

    atomic<bool> running; // Flag to control thread loop

public:
    // Pool of client threads
    vector<thread> threads;

    // Creates and starts threads
    Threadpool(size_t num_threads, const string &server_addr, int server_port, const string &file);

    // Stops all threads gracefully
    ~Threadpool();

    // Thread function
    void sendRequest();
};
