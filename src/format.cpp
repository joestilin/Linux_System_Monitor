//#include <string>
#include "format.h"

using std::string;

// Helper function returns string representation of time
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) { 
    string time = "";
    int hrs = seconds / 3600;
    int mins = (seconds % 3600) / 60;
    int secs = (seconds % 3600) % 60;

    // format return string as HH:MM:SS
    if (hrs < 10) {
        time = time + "0";
    }
    time = time + std::to_string(hrs) + ":"; 
    if (mins < 10) {
        time = time + "0";
    }
    time = time + std::to_string(mins) + ":";
    if (secs < 10) {
        time = time + "0";
    }
    time = time + std::to_string(secs);
    return time; 
}