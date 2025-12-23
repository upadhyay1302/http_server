#!/usr/bin/env bash
set -e

SERVER_BIN="./http_server_multithreading/build/server"
PORT=10000
BENCH_DIR="benchmarks"
OUT_FILE="$BENCH_DIR/multithread_static_ab.txt"

REQS=10000
CONCURRENCY=8

mkdir -p "$BENCH_DIR"

echo "Starting HTTP server..."
$SERVER_BIN &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"

# Give server time to start
sleep 1

echo "Running ApacheBench..."
ab -n $REQS -c $CONCURRENCY http://127.0.0.1:$PORT/ > "$OUT_FILE" || true

echo "Benchmark complete. Results saved to $OUT_FILE"

echo "Stopping server..."
kill $SERVER_PID || true
wait $SERVER_PID 2>/dev/null || true

echo "Done."
