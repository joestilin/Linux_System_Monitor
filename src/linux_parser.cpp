#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// Helper function that reads value from file system given key
template <typename T>
T findValueByKey(std::string const &keyfilter, std::string const &filename) {
  std::string line;
  std::string key;
  T value; 

  std::ifstream stream(filename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while(linestream >> key >> value) {
        if (key == keyfilter) {
          stream.close();
          return value;
        }
      }
    }
  }
  return value; 
}

// Helper function to read the first value in a file
template <typename T>
T findValue(std::string const& filename) {
  std::string line;
  T value;

  std::ifstream stream(filename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value;
  } 
  return value;
}

// read the operating system name from the file system
// From file: /etc/os-release
string LinuxParser::OperatingSystem() {
  std::string line;
  std::string key;
  std::string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// Read the kernel name from the file system
// From file: /proc/version
string LinuxParser::Kernel() {
  std::string os;
  std::string version;
  std::string kernel;
  std::string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  stream.close();

  return kernel;
}

// Get a vector of currently running process ids
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Reads and returns the system memory utilization as a percentage
// From file: /proc/meminfo
float LinuxParser::MemoryUtilization() {
  float total;
  float free;

  total = findValueByKey<float>(filterMemTotalString, 
                                kProcDirectory + kMeminfoFilename);
  free = findValueByKey<float>(filterMemFreeString, 
                               kProcDirectory + kMeminfoFilename);

  return (total - free) / total;
}

// Read and return the total system up time
// From file: /proc/uptime
long LinuxParser::UpTime() { 
  long uptime;
  uptime = findValue<long>(kProcDirectory + kUptimeFilename);
  return uptime; 
}

// Return the total number of jiffies for the system
long LinuxParser::Jiffies() { 
  long total_jiffies;
  long user; 
  long nice;
  long system;
  long idle; 
  long iowait; 
  long irq; 
  long softirq; 
  long steal;
  std::vector<string> utilizations = LinuxParser::CpuUtilization();
  
  user = stol(utilizations[kUser_]);
  nice = stol(utilizations[kNice_]);
  system = stol(utilizations[kSystem_]);
  idle = stol(utilizations[kIdle_]);
  iowait = stol(utilizations[kIOwait_]);
  irq = stol(utilizations[kIRQ_]);
  softirq = stol(utilizations[kSoftIRQ_]);
  steal = stol(utilizations[kSteal_]);

  total_jiffies = user + nice + system + idle + iowait + irq + softirq + steal;

  return total_jiffies;
}

// Read and return the number of active jiffies for a PID
// From file: /proc/[PID]/stat
// Formula: total active jiffies = utime + stime + cutime + cstime
long LinuxParser::ActiveJiffies(int pid) {
  long active_jiffies;
  long utime;
  long stime;
  long cutime;
  long cstime;
  std::string line;
  std::string value;
  std::vector<string> values;

  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for (int i = 0; i < 22; i++) {
      linestream >> value;
      values.emplace_back(value);
    }
  
    utime = stol(values[13]);
    stime = stol(values[14]);
    cutime = stol(values [15]);
    cstime = stol(values[16]);

    active_jiffies = utime + stime + cutime + cstime;
  }
  else { active_jiffies = 0; }

  return active_jiffies;
}

// Return the number of active jiffies for the system
// Formula: total active = user + nice + system + irq + softirq + steal
long LinuxParser::ActiveJiffies() {
  long active_jiffies;
  long user; 
  long nice; 
  long system; 
  long irq; 
  long softirq; 
  long steal;
  std::vector<string> utilizations = LinuxParser::CpuUtilization();

  user = stol(utilizations[kUser_]);
  nice = stol(utilizations[kNice_]);
  system = stol(utilizations[kSystem_]);
  irq = stol(utilizations[kIRQ_]);
  softirq = stol(utilizations[kSoftIRQ_]);
  steal = stol(utilizations[kSteal_]);

  active_jiffies = user + nice + system + irq + softirq + steal;

  return active_jiffies;
}

// Return the number of idle jiffies for the system
// formula: total idle = idle + iowait
long LinuxParser::IdleJiffies() {
  long idle_jiffies;
  long idle;
  long iowait;
  std::vector<string> utilizations = LinuxParser::CpuUtilization();
  idle = stol(utilizations[kIdle_]);
  iowait = stol(utilizations[kIOwait_]);

  idle_jiffies = idle + iowait;
  
  return idle_jiffies;
}

// Read and return a vector CPU utilizations in order:
// [user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice]
vector<string> LinuxParser::CpuUtilization() { 
  vector<string> utilization;
  string cpu;
  string line;
  string value;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    while(linestream >> value) {
      utilization.emplace_back(value);
    }
  }
  return utilization;
}

// Read and return the total number of processes
// From file: /proc/stat
int LinuxParser::TotalProcesses() { 
  int total_processes;
  total_processes = findValueByKey<int>(filterProcesses,
                                        kProcDirectory + kStatFilename);
  return total_processes;
}

// Read and return the number of running processes
// From file: /proc/stat
int LinuxParser::RunningProcesses() { 
  int running_processes;
  running_processes = findValueByKey<int>(filterRunningProcesses,
                                        kProcDirectory + kStatFilename);
  return running_processes;
 }

// Read and return the command associated with a process
// From file: /proc/[PID]/cmdline
std::string LinuxParser::Command(int pid) {
  std::string command;
  command = findValue<std::string>(kProcDirectory + to_string(pid)
                                    + kCmdlineFilename);
  return command;
}

// Read and return the memory used by a process
// From file: /proc/[PID]/status
std::string LinuxParser::Ram(int pid) { 
  std::string ram;
  ram = findValueByKey<std::string>(filterProcMem,
                                    kProcDirectory + to_string(pid) 
                                    + kStatusFilename);
  ram = to_string(stol(ram) / static_cast<long>(1000));
  return ram;
}

// Read and return the user ID associated with a process
// From file: /proc/[PID]/status
std::string LinuxParser::Uid(int pid) { 
  std::string uid;
  uid = findValueByKey<std::string>(filterUID,
                                    kProcDirectory + to_string(pid) 
                                    + kStatusFilename);
  return uid; 
 }

// Read and return the user associated with a process
// From file: /etc/passwd
std::string LinuxParser::User(int pid) {
  std::string user;
  std::string uid = LinuxParser::Uid(pid);
  std::string line; 
  std::string value; 
  std::string x; 
  std::string uid_cand;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream linestream(line);
        linestream >> value >> x >> uid_cand;
        if (uid_cand == uid) {
          user = value;
        }
      }
    }
    else {user = ""; }

  return user; 
}

// Read and return the uptime of a process
// From file: /proc/[PID]/stat
// Formula: (system uptime) - (process startime)
long LinuxParser::UpTime(int pid) {
  long proc_uptime; 
  long sys_uptime;
  std::string line;
  std::string value;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for (int i = 0; i < 22; i++) {
      linestream >> value;
    }
    sys_uptime = LinuxParser::UpTime();
    proc_uptime = sys_uptime - long(stol(value) / sysconf(_SC_CLK_TCK));
  }
  else { proc_uptime = 0; }
  
  return proc_uptime;
  }