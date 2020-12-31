#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

//#include "process.h"
//#include "processor.h"
#include "system.h"

using namespace std;

// Return the system's CPU
Processor& System::Cpu() { return cpu_; }

/*  Return a container composed of the system's processes.

    On each call of this function, the vector of Process objects
    is compared with the current processes on the system.
    New processes are pushed to the vector, and killed processes
    are erased from the vector.
*/
vector<Process>& System::Processes() { 
    
    // fill an int vector of old pids from the last call
    std::vector<int> old_pids;
    for (auto & proc : processes_) {
        old_pids.emplace_back(proc.Pid());
    }
    // vector of new pids
    std::vector<int> new_pids = LinuxParser::Pids();

    // sort these vectors
    std::sort(old_pids.begin(), old_pids.end());
    std::sort(new_pids.begin(), new_pids.end());

    // their difference: deleted and added pids
    std::vector<int> deleted_pids;
    std::vector<int> added_pids;

    // added pids
    std::set_difference(new_pids.begin(), new_pids.end(), 
                        old_pids.begin(), old_pids.end(), 
                        std::inserter(added_pids, added_pids.begin()));
    
    // killed pids
    std::set_difference(old_pids.begin(), old_pids.end(), 
                        new_pids.begin(), new_pids.end(), 
                        std::inserter(deleted_pids, deleted_pids.begin()));

    // add newly started processes to processes_ vector
    for (int pid : added_pids) {
        Process proc;
        proc.setPid(pid);
        processes_.emplace_back(proc);
    }
    // erase newly killed processes from the processes_ vector
    for (u_int i = 0; i < processes_.size(); i++) {
        for (int pid : deleted_pids) {
            if (processes_[i].Pid() == pid) {
                processes_.erase(processes_.begin() + i);
            }
        }
    }
    // Sort list of processes
    std::sort(processes_.begin(), processes_.end(),
                [](const auto & p1, const auto & p2) {return p1 < p2; });
    
    return processes_;
}

// Return the system's kernel identifier (string)
std::string System::Kernel() { 
    return LinuxParser::Kernel();
}

// Return the system's memory utilization
float System::MemoryUtilization() { 
    return LinuxParser::MemoryUtilization();
}

// Return the operating system name
std::string System::OperatingSystem() { 
    return LinuxParser::OperatingSystem(); 
}

// Return the number of processes actively running on the system
int System::RunningProcesses() { 
    return LinuxParser::RunningProcesses(); 
}

// Return the total number of processes on the system
int System::TotalProcesses() { 
    return LinuxParser::TotalProcesses(); 
}

// Return the number of seconds since the system started running
long int System::UpTime() { 
    return LinuxParser::UpTime();
    
}