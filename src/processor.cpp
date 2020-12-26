#include "processor.h"
#include <string>

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() 
{ 
    float utilization;
    long current_total;
    long current_active;
    long d_total;
    long d_active;

    current_total = LinuxParser::Jiffies();
    current_active = LinuxParser::ActiveJiffies();
    d_total = current_total - prev_total_;
    d_active = current_active - prev_active_;

    // update private members
    prev_total_ = current_total;
    prev_active_ = current_active;

    utilization = (float)(d_active) / d_total;
    return utilization;

}