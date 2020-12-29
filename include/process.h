#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include "linux_parser.h"

/*
Basic class for Process representation
*/
class Process {
 public:
  void setPid(int pid);
  int Pid();                               
  std::string User();                   
  std::string Command();                   
  float CpuUtilization();                 
  std::string Ram();                      
  long int UpTime();                       
  bool operator<(Process const& a) const;

 private:
    int pid_;
    long prev_active_{0};
    long prev_uptime_{0};
    float cpu_utilization_{0.0};
};

#endif