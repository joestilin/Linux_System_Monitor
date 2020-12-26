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

// DONE: An example of how to read data from the filesystem
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

// DONE: An example of how to read data from the filesystem
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

// BONUS: Update this to use std::filesystem
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

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {

  // parse MemInfo File
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
      //if (key == "MemAvailable:") {available = stof(value); }
      //if (key == "Buffers:") {buffers = stof(value); }
    }
  }

  utilization = (total - free) / total;

   return utilization; 
}

// TODO: Read and return the system uptime
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

// TODO: Read and return the number of jiffies for the system
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

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
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
long LinuxParser::IdleJiffies() {
  long idle_jiffies;
  long idle, iowait;
  std::vector<string> utilizations = LinuxParser::CpuUtilization();
  idle = stol(utilizations[3]);
  iowait = stol(utilizations[4]);

  idle_jiffies = idle + iowait;
  
  return idle_jiffies;
}

// Read and return CPU utilization
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

// TODO: Read and return the total number of processes
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

// TODO: Read and return the number of running processes
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

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string command;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, command);
  }
  return command; 
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid[[maybe_unused]]) { return 0; }