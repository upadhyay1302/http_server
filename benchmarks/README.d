# Benchmarks

This directory contains performance benchmark results for the HTTP server
implementation using multithreading and epoll-based event loop.

The benchmarks are intended to provide a **reference comparison** of server
behavior under load rather than absolute performance guarantees.

## Tooling
- **wrk** — a modern HTTP benchmarking tool designed for high concurrency

## Workload
- Static file: `index.html` (~87 bytes)
- Endpoint: `GET /`
- Host: `http://127.0.0.1:10000`
- Duration: 20 seconds per test
- Threads: 8
- Connections: 10,000

## Methodology
- Benchmarks were executed locally using `wrk`
- The server was running on localhost during testing
- Each benchmark run was scripted to ensure repeatability

## Results
- Benchmark outputs for each run are stored as text files in this directory
- Each results file contains the full `wrk` output, including:
  - Latency statistics
  - Requests per second
  - Total requests handled
  - Transfer rates
  - Socket error information (if any)

## Notes
- Tests were run on localhost, real-world performance will vary
- Results depend on hardware, OS scheduling, and system load
- These benchmarks are provided for comparison and documentation purposes
