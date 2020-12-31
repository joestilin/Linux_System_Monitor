#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"

using std::string;
using std::to_string;
using std::vector;

// set this processes' ID
void Process::setPid(int pid) {
    pid_ = pid;
}

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
// Calculated since the last time this function was called
float Process::CpuUtilization() { 
    long current_active;
    long current_uptime;
    long d_active;
    long d_uptime;

    current_active = LinuxParser::ActiveJiffies(pid_);
    current_uptime = LinuxParser::UpTime(pid_);

    // difference in active jiffies
    d_active = current_active - prev_active_;

    // difference in time
    d_uptime = current_uptime - prev_uptime_;

    // update
    prev_active_ = current_active;
    prev_uptime_ = current_uptime;

    if (d_uptime != 0) {
        cpu_utilization_ = (static_cast<float>(d_active) / sysconf(_SC_CLK_TCK))
                         / static_cast<float>(d_uptime);
    }
    
    return cpu_utilization_;
}

// Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid_); }

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid_); }

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// Overload the "less than" comparison operator for Process objects
// Compare by cpu utilization
bool Process::operator<(Process const& a) const {
return a.cpu_utilization_ < cpu_utilization_;
}