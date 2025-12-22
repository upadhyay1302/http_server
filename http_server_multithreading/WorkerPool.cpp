#include "include/WorkerPool.h"
#include "include/ThreadSafeCout.h"
#include "include/request.h"
#include "include/SocketUtils.h"

#include <chrono>

using namespace std;

// Constructor: initialize the thread pool
ThreadPool::ThreadPool(size_t initialThreads,
                       size_t bufferSize,
                       size_t minThr,
                       size_t maxThr)
    : queueSize(bufferSize),
      minThreads(minThr),
      maxThreads(maxThr)
{
    for (size_t i = 0; i < initialThreads; ++i) {
        threads.emplace_back(&ThreadPool::threadLoop, this);
        liveThreads++;
        ThreadSafeCout() << "[Init] Thread created: "
                     << threads.back().get_id() << endl;
    }
}

// Worker thread loop: fetch and process jobs
void ThreadPool::threadLoop() {
    while (true) {
        int fd = -1;

        {
            unique_lock<mutex> lock(queueMutex);

            // Wait for a job or timeout
            if (!jobAvailable.wait_for(
                    lock,
                    chrono::seconds(10),
                    [this] { return !jobs.empty(); })) {

                // Exit idle threads above minimum
                if (liveThreads.load() > minThreads) {
                    liveThreads--;
                    ThreadSafeCout() << "[Thread "
                                 << this_thread::get_id()
                                 << "] idle timeout → exiting"
                                 << endl;
                    return;
                }
                continue;
            }

            // Pop a job from the queue
            fd = jobs.front();
            jobs.pop();
            spaceAvailable.notify_one();
        }

        // Process the job
        activeThreads++;
        processJob(fd);
        activeThreads--;
        totalRequests++;
    }
}

// Add a job to the queue
void ThreadPool::queueJob(int fd) {
    unique_lock<mutex> lock(queueMutex);

    // Wait if the queue is full
    spaceAvailable.wait(lock, [this] {
        return jobs.size() < queueSize;
    });

    jobs.push(fd);

    ThreadSafeCout() << "[Main] Added FD=" << fd
                 << " to queue, size=" << jobs.size()
                 << endl;

    jobAvailable.notify_one();

    // Dynamic scaling: spawn a new thread if needed
    if (liveThreads.load() < maxThreads &&
        jobs.size() > liveThreads.load()) {

        threads.emplace_back(&ThreadPool::threadLoop, this);
        liveThreads++;

        ThreadSafeCout() << "[Scale] Spawned thread "
                     << threads.back().get_id()
                     << " due to load"
                     << endl;
    }
}

// Execute a single job
void ThreadPool::processJob(int fd) {
    auto start = chrono::high_resolution_clock::now();

    handle_request(fd); // Call existing request handler

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(
        end - start).count();

    ThreadSafeCout() << "[Thread "
                 << this_thread::get_id()
                 << "] completed FD=" << fd
                 << " in " << duration << " ms"
                 << endl;

    // Close the file descriptor
    // close_or_die(fd);  // Uncomment if needed
}

// Destructor: join all threads
ThreadPool::~ThreadPool() {
    for (auto& t : threads) {
        if (t.joinable())
            t.join();
    }
}

// Metrics accessors
size_t ThreadPool::getQueueSize() {
    lock_guard<mutex> lock(queueMutex);
    return jobs.size();
}

size_t ThreadPool::getActiveThreads() {
    return activeThreads.load();
}

size_t ThreadPool::getTotalRequests() {
    return totalRequests.load();
}

size_t ThreadPool::getLiveThreads() {
    return liveThreads.load();
}
