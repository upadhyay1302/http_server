#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

using namespace std;

/*
  ThreadPool class for managing a pool of worker threads.

  - Accepts incoming jobs (file descriptors) and distributes them
    to worker threads.
  - Supports dynamic scaling of threads based on load.
  - Maintains runtime metrics: active threads, live threads, total requests, queue size.
*/
class ThreadPool {
public:
    ThreadPool(size_t initialThreads,
               size_t bufferSize,
               size_t minThreads = 1,
               size_t maxThreads = 16);

    ~ThreadPool();

    // Queue a new job (file descriptor)
    void queueJob(int fd);

    // ---- Metrics accessors ----
    size_t getQueueSize();       // Current queue size
    size_t getActiveThreads();   // Threads currently processing jobs
    size_t getTotalRequests();   // Total jobs processed
    size_t getLiveThreads();     // Threads currently alive

private:
    void threadLoop();           // Worker thread main loop
    void processJob(int fd);     // Execute a single job

    // Worker threads
    vector<thread> threads;

    // Job queue
    queue<int> jobs;

    // Synchronization primitives
    mutex queueMutex;
    condition_variable jobAvailable;
    condition_variable spaceAvailable;

    // Limits
    size_t queueSize;
    size_t minThreads;
    size_t maxThreads;

    // Runtime metrics
    atomic<size_t> liveThreads{0};      // Number of threads currently alive
    atomic<size_t> activeThreads{0};    // Threads actively processing jobs
    atomic<size_t> totalRequests{0};    // Total jobs processed
};
