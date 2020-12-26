#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

//#include "process.h"
//#include "processor.h"
#include "system.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// TODO: Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// TODO: Return a container composed of the system's processes
// Use LinuxParser::Pids() to get vector of Pids
// For each one, instantiate a Process and push it back on to
// a vector, and return the vector.
vector<Process>& System::Processes() { 
    vector<Process> existing_processes{};
    for (int pid : LinuxParser::Pids()) {
        Process process{};
        process.setPid(pid);
        existing_processes.emplace_back(process);
    }
    processes_ = existing_processes;
    return processes_;
}

// DONE: Return the system's kernel identifier (string)
std::string System::Kernel() { 
    return LinuxParser::Kernel();
}

// DONE: Return the system's memory utilization
float System::MemoryUtilization() { 
    return LinuxParser::MemoryUtilization();
}

// DONE: Return the operating system name
std::string System::OperatingSystem() { 
    return LinuxParser::OperatingSystem(); 
}

// DONE: Return the number of processes actively running on the system
int System::RunningProcesses() { 
    return LinuxParser::RunningProcesses(); 
}

// DONE: Return the total number of processes on the system
int System::TotalProcesses() { 
    return LinuxParser::TotalProcesses(); 
}

// DONE: Return the number of seconds since the system started running
long int System::UpTime() { 
    return LinuxParser::UpTime();
    
}