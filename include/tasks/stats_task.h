#pragma once
#include "ui.h"
#include "mbed_stats.h"

namespace Tasks {

class Stats {
public:
  Stats();
  void setDisplay(UI::Display *display);
  void start();
  void stop();
};

} // namespace Tasks