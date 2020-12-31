#include "processor.h"
#include <string>

// Return the aggregate CPU utilization
// Calculated since the last time this function was called
float Processor::Utilization() 
{ 
    float utilization;
    long current_total;
    long current_active;
    long d_total;
    long d_active;

    current_total = LinuxParser::Jiffies();
    current_active = LinuxParser::ActiveJiffies();

    // difference in total jiffies
    d_total = current_total - prev_total_;

    // difference in active jiffies
    d_active = current_active - prev_active_;

    // update
    prev_total_ = current_total;
    prev_active_ = current_active;
    
    utilization = static_cast<float>(d_active)
                / static_cast<float>(d_total);
    
    return utilization;

}