#include "Thread.h"
#include "mbed.h"
#include "tasks/stats_task.h"
#include "ui.h"
#include <cstdio>

using namespace Tasks;

#define SAMPLE_TIME_MS 1000
#define MAX_THREAD_STACK 384

UI::Display *_display;
EventQueue *stats_queue = mbed_event_queue();
int _thread_id;
Thread *_thread;

void Stats::setDisplay(UI::Display *display) {
    _display = display;
}

void collect_stats() {
  mbed_stats_cpu_t stats;
  mbed_stats_cpu_get(&stats);
  _display->update_stats(&stats, SAMPLE_TIME_MS);
}

void Stats::start() {
  if (_thread) {
    // we're already running
    return;
  }

  _thread_id = stats_queue->call_every(SAMPLE_TIME_MS, collect_stats);

  _thread = new Thread(osPriorityNormal, MAX_THREAD_STACK);
}

void Stats::stop() {
  if (_thread) {
    _thread->terminate();
  }
}