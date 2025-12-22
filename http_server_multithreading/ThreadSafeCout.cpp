#include "include/ThreadSafeCout.h"

using namespace std;

// Support manipulators like endl, flush, etc.
ThreadSafeCout& ThreadSafeCout::operator<<(ostream& (*manip)(ostream&)) {
    buffer << manip;
    return *this;
}

// On destruction, dump the entire buffer to cout at once
ThreadSafeCout::~ThreadSafeCout() {
    cout << buffer.str();
}
