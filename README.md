# HTTP Server (C++)

A high-performance HTTP server written in modern C++, focused on understanding concurrency models, networking, and systems programming from the ground up.

This repository currently contains a multithreaded HTTP server implementation along with a client-side load generator used for testing and benchmarking.

## 📁 Project Structure

```
http_server/
├── http_server_multithreading/    # Multithreaded HTTP server implementation
│   ├── include/                   # Header files
│   ├── build/                     # Build artifacts (ignored)
│   ├── CMakeLists.txt
│   └── *.cpp
│
├── client/                        # Client-side load testing tools
│   ├── Makefile
│   ├── client.cpp
│   ├── client_helper.*
│   └── client_threadpool.*
│
├── .gitignore
└── README.md
```

## 🚀 Features Implemented

### 🧵 Multithreaded HTTP Server

- Thread pool–based request handling
- Bounded request queue to prevent overload
- Worker threads consuming connections concurrently
- Proper synchronization using mutexes & condition variables
- Graceful handling of concurrent client connections
- Clean separation of concerns:
  - Connection handling
  - Request parsing
  - Response generation

### 📊 /metrics Endpoint

- Exposes server runtime statistics (e.g. total requests handled)
- Thread-safe metric updates
- Useful for debugging and performance analysis

### 🧪 Client Load Generator

- Multi-threaded HTTP client for stress testing
- Configurable:
  - Target host
  - Port
  - Request path
  - Number of client threads
- Repeated request execution to simulate real-world load

## 🛠️ Build & Run

### Build the Server

```bash
cd http_server_multithreading
mkdir build && cd build
cmake ..
make
```

### Run the Server

```bash
./server -t <num_threads> -b <buffer_size>
```

**Example:**

```bash
./server -t 4 -b 16
```

### Build & Run the Client

```bash
cd client
make
./client -h localhost -p 10000 -f /metrics -t 8
```

## 📌 Design Goals

- Learn low-level networking using POSIX sockets
- Understand thread pools vs event-driven models
- Practice safe concurrent programming
- Build infrastructure similar to real production servers

## 🔜 Next Steps (Work in Progress)

This project will be extended with:

### ⚡ Reactor-based server using epoll

- Non-blocking I/O
- Event loop architecture
- Improved scalability under high concurrency
- Cleaner abstraction for connection lifecycle

The reactor implementation will live in a separate module and is currently under active development.

## 🧠 Notes

This project is intentionally built without external web frameworks to deeply understand how HTTP servers work internally.

## 📜 License

MIT License