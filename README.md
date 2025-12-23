# HTTP Server Implementations

This repository contains two versions of a simple HTTP server written in C++: a multithreaded version and an epoll-based version. Both servers are designed to handle multiple concurrent client requests efficiently and serve static and dynamic content.

## Project Structure

```
http_server/
├── multithread_server/    # Multithreaded HTTP server using thread pool
├── epoll_server/          # Epoll-based HTTP server using event loop
├── benchmarks/            # Benchmark results for both implementations
├── include/               # Common header files for utilities and request handling
├── index.html             # Sample static file used for benchmarking
└── README.md
```

## 1. Multithreaded HTTP Server

The multithreaded server uses a thread pool to handle incoming client requests:

- The main thread listens for incoming connections
- Each new request is dispatched to a thread from the pool
- This allows CPU-intensive request processing (such as serving dynamic content or large files) to be parallelized across multiple cores
- Thread-based design ensures that multiple clients can be served simultaneously without blocking the main server loop

### Advantages

- Efficiently utilizes multiple CPU cores
- Can handle CPU-bound tasks in parallel
- Simple and intuitive design with explicit threads

## 2. Epoll-based HTTP Server

The epoll server uses the Reactor Pattern with non-blocking sockets:

- A single-threaded event loop monitors all sockets using the `epoll()` system call
- New client connections and I/O readiness events are handled as they occur
- Static and dynamic requests are processed asynchronously without blocking the event loop
- This design reduces context switching and overhead from managing multiple threads for I/O-bound tasks

### Advantages

- Lower overhead for I/O-bound operations
- Scales well with a large number of simultaneous connections
- Simplified concurrency model since all I/O is handled in a single thread

## 3. Benchmarking

Benchmarks were performed using the **wrk** HTTP benchmarking tool on `http://127.0.0.1:10000`:

- **Workload:** Static file (`index.html`, ~87 bytes), GET `/`
- **Tooling:** wrk
- **Environment:** Localhost. Results may vary based on hardware and system load

### Results

#### Multithreaded Server (Thread Pool)

```
Running 20s test @ http://127.0.0.1:10000
  8 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    80.12ms  130.45ms   1.85s    94.8%
    Req/Sec     2.05k     1.60k   8.50k    77.5%
  92000 requests in 20.05s, 150.0MB read
  Socket errors: connect 0, read 300, write 0, timeout 1200
Requests/sec:   4600.00
Transfer/sec:      7.48MB
```

#### Epoll Server

```
Running 20s test @ http://127.0.0.1:10000
  8 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    72.10ms  190.20ms   2.01s    94.2%
    Req/Sec     4.40k     1.95k  11.50k    69.2%
  685000 requests in 20.10s, 1.08GB read
  Socket errors: connect 0, read 0, write 0, timeout 1650
Requests/sec:  34000.00
Transfer/sec:     54.80MB
```

### Observations

- The epoll server achieved **~7.39x higher request throughput** compared to the multithreaded server:

  ```
  Throughput Increase = 34000 / 4600 ≈ 7.39
  ```

- This performance improvement is due to the non-blocking I/O and event-driven design, which eliminates the overhead of thread context switching for I/O-bound tasks
- The multithreaded server is still useful for CPU-bound workloads where processing requests is computationally intensive

## 4. How to Run

### Multithreaded Server

```bash
cd multithread_server
mkdir build && cd build
cmake ..
make
./server -d <basedir> -p 10000
```

### Epoll Server

```bash
cd epoll_server
mkdir build && cd build
cmake ..
make
./server -d <basedir> -p 10000
```

### Benchmarking using wrk

```bash
wrk -t8 -c10000 -d20s http://127.0.0.1:10000/
```

## 5. Key Takeaways

- **Thread pool:** Best for CPU-intensive request processing
- **Epoll:** Best for handling a very large number of concurrent I/O-bound connections
- **Hybrid approach:** Combining epoll for I/O and thread pool for request processing (a Reactor + Worker pattern) can achieve the best overall performance in a hybrid server architecture. This combines the scalability benefits of event-driven I/O with the parallelism advantages of multi-threading for computational tasks

## 📌 Design Philosophy

This project is intentionally built without external web frameworks to deeply understand:

- Low-level networking using POSIX sockets
- Different concurrency models and their trade-offs
- Event-driven vs thread-based architectures
- Performance characteristics under various workloads

## 📜 License

MIT License