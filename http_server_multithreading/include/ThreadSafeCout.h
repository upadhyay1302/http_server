#pragma once

#include <iostream>
#include <sstream>

using namespace std;

/*
   ThreadSafeCout stream for thread-safe printing.

  Normally, using std::cout like:
      cout << "Hello " << first << " " << last << endl;
  may interleave in multithreaded scenarios because each << is separate.

  This class collects the entire output into a string buffer first,
  then flushes it to std::cout in one atomic operation when the object
  goes out of scope.
*/

class ThreadSafeCout {
private:
    ostringstream buffer;

public:
    // Overload for any type that can be streamed
    template <typename T>
    ThreadSafeCout& operator<<(const T& val);

    // Overload for standard stream manipulators (like endl)
    ThreadSafeCout& operator<<(ostream& (*manip)(ostream&));

    // Flush buffer to cout on destruction
    ~ThreadSafeCout();
};

// Templated functions must be in header
template <typename T>
ThreadSafeCout& ThreadSafeCout::operator<<(const T& val) {
    buffer << val;
    return *this;
}
