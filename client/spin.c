#include <iostream>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <thread>

double get_seconds() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration<double>(duration).count();
}

int main() {
    // Get spin duration from QUERY_STRING
    double spin_for = 0.0;
    const char* query = std::getenv("QUERY_STRING");
    if (query) {
        spin_for = std::atof(query); // safer than atoi for floating point
    }

    // Spin for the requested duration
    double t1 = get_seconds();
    while ((get_seconds() - t1) < spin_for) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    double t2 = get_seconds();

    // Generate HTML response
    std::string content;
    content += "<p>Welcome to the CGI program (" + std::string(query ? query : "") + ")</p>\r\n";
    content += "<p>My only purpose is to waste time on the server!</p>\r\n";
    content += "<p>I spun for " + std::to_string(t2 - t1) + " seconds</p>\r\n";

    // Output HTTP response
    std::cout << "Content-Length: " << content.size() << "\r\n";
    std::cout << "Content-Type: text/html\r\n\r\n";
    std::cout << content;
    std::cout.flush();

    return 0;
}
