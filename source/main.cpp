#include "Callback.h"
#include "EventQueue.h"
#include "ThisThread.h"
#include "database.h"
#include "mbed.h"
#include "mbed_wait_api.h"
#include "ui/ui.h"
#include "webserver.h"
#include <cstdlib>

#define SD_ACTIVITY_CHECK_RATE 500

Thread thread_main;
Thread thread_web_server;

EventQueue queue(32 * EVENTS_EVENT_SIZE);

void webserver_task(WebServer *webServer) {
  while (true) {
    webServer->tick();
  }
}

int main(void) {
  thread_main.start(callback(&queue, &EventQueue::dispatch_forever));
  printf("[thread_main]: starting in context %p\r\n", thread_main.get_id());



  UI::Display display;
  display.init();

  Database db;
  WebServer webServer = WebServer(&db);

  display.log((uint8_t *)"[webserver]: starting");

  int status = webServer.start();
  if (status == 0) {
    display.log((uint8_t *)"[webserver]: error - No network interface found");
    exit(1);
  }

  display.log((uint8_t *)"[webserver]: IP: 192.168.1.100");

  display.log((uint8_t *)"[host]: spawning webserver_thread");
  thread_web_server.start(callback(webserver_task, &webServer));
}
