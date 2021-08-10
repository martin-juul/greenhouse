#include "Callback.h"
#include "EventQueue.h"
#include "ThisThread.h"
#include "database.h"
#include "mbed.h"
#include "mbed_wait_api.h"
#include "ui/ui.h"
#include "webserver.h"
#include <cstdlib>

#define MAX_THREAD_STACK 384
#define SAMPLE_TIME_MS   1000
#define LOOP_TIME_MS     3000

Thread thread_main;
Thread thread_web_server;
UI::Display *display;

EventQueue queue(32 * EVENTS_EVENT_SIZE);

void webserver_task(WebServer *webServer) {
  while (true) {
    webServer->tick();
  }
}

void print_cpu_stats()
{
    mbed_stats_cpu_t stats;
    mbed_stats_cpu_get(&stats);
  
    display->update_stats(&stats, SAMPLE_TIME_MS);
}


int main(void) {
  thread_main.start(callback(&queue, &EventQueue::dispatch_forever));
  printf("[thread_main]: starting in context %p\r\n", thread_main.get_id());

  display->init();

  int id;
  Thread *thread_stats;
  EventQueue *stats_queue = mbed_event_queue();
  id = stats_queue->call_every(SAMPLE_TIME_MS, print_cpu_stats);
  thread_stats = new Thread(osPriorityNormal, MAX_THREAD_STACK);

  Database db;
  WebServer webServer = WebServer(&db);

  display->log((uint8_t *)"[webserver]: starting");

  int status = webServer.start();
  if (status == 0) {
    display->log((uint8_t *)"[webserver]: error - No network interface found");
    exit(1);
  }

  display->log((uint8_t *)"[webserver]: IP: 192.168.1.100");

  display->log((uint8_t *)"[host]: spawning webserver_thread");
  thread_web_server.start(callback(webserver_task, &webServer));
}
