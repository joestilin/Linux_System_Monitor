#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "linux_parser.h"
#include <string>

class Processor {
 public:
  float Utilization();  // TODO: See src/processor.cpp

  // TODO: Declare any necessary private members
 private:
 long prev_active_{0};
 long prev_total_{0};
};

#endif