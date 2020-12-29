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

// read the operating system name from the file system
// From file: /etc/os-release
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
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
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
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
  float total, free;
  float utilization;
  string line;
  string key, value;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while(std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "MemTotal:") { total = stof(value); }
      if (key == "MemFree:") { free = stof(value); }
    }
  }
  utilization = (total - free) / total;
  return utilization; 
}

// Read and return the total system up time
// From file: /proc/uptime
long LinuxParser::UpTime() { 
  string time;
  string line;
  long uptime;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> time;
  }
  uptime = stol(time);
  return uptime; 
}

// Return the total number of jiffies for the system
long LinuxParser::Jiffies() { 
  long total_jiffies;
  long user, nice, system, idle, iowait, irq, softirq, steal;
  std::vector<string> utilizations = LinuxParser::CpuUtilization();
  
  user = stol(utilizations[0]);
  nice = stol(utilizations[1]);
  system = stol(utilizations[2]);
  idle = stol(utilizations[3]);
  iowait = stol(utilizations[4]);
  irq = stol(utilizations[5]);
  softirq = stol(utilizations[6]);
  steal = stol(utilizations[7]);

  total_jiffies = user + nice + system + idle + iowait + irq + softirq + steal;

  return total_jiffies;
}

// Read and return the number of active jiffies for a PID
// From file: /proc/[PID]/stat
// Formula: total active jiffies = utime + stime + cutime + cstime
long LinuxParser::ActiveJiffies(int pid) {
  long active_jiffies;
  long utime, stime, cutime, cstime;
  string line, value;
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
  long user, nice, system, irq, softirq, steal;
  std::vector<string> utilizations = LinuxParser::CpuUtilization();

  user = stol(utilizations[0]);
  nice = stol(utilizations[1]);
  system = stol(utilizations[2]);
  irq = stol(utilizations[5]);
  softirq = stol(utilizations[6]);
  steal = stol(utilizations[7]);

  active_jiffies = user + nice + system + irq + softirq + steal;

  return active_jiffies;
}

// Return the number of idle jiffies for the system
// formula: total idle = idle + iowait
long LinuxParser::IdleJiffies() {
  long idle_jiffies;
  long idle, iowait;
  std::vector<string> utilizations = LinuxParser::CpuUtilization();
  idle = stol(utilizations[3]);
  iowait = stol(utilizations[4]);

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
  string key, value;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "processes") { total_processes = stol(value); }
    }
  }
  return total_processes;
}

// Read and return the number of running processes
// From file: /proc/stat
int LinuxParser::RunningProcesses() { 
  int running_processes;
    string key, value;
    string line;
    std::ifstream stream(kProcDirectory + kStatFilename);
    if (stream.is_open()) {
      while (std::getline(stream, line)) {
        std::istringstream linestream(line);
        linestream >> key >> value;
        if (key == "procs_running") { running_processes = stol(value); }
      }
    }
    return running_processes;
 }

// Read and return the command associated with a process
// From file: /proc/[PID]/cmdline
string LinuxParser::Command(int pid) {
  string command;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, command);
  }
  else { command = ""; }

  return command; 
}

// Read and return the memory used by a process
// From file: /proc/[PID]/status
string LinuxParser::Ram(int pid) { 
  string ram;
  string line, key, value;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "VmSize:") {
        ram = value;
      }
    }
  
  // convert from kB to MB
    ram = to_string(stol(ram) / (long)1000);
  }
  else { ram = ""; }

  return ram;
}

// Read and return the user ID associated with a process
// From file: /proc/[PID]/status
string LinuxParser::Uid(int pid) { 
  string uid;
  string line, key, value;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "Uid:") {
        uid = value;
      }
    }
  }
  else { uid = ""; }

  return uid; 
 }

// Read and return the user associated with a process
// From file: /etc/passwd
string LinuxParser::User(int pid) {
  string user;
  string uid = LinuxParser::Uid(pid);
  string line, value, x, uid_cand;
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
  string line, value;
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