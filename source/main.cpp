#include "Callback.h"
#include "database.h"
#include "jukebox_task.h"
#include "mbed.h"
#include "mbed_wait_api.h"
#include "storage_task.h"
#include "ui/ui.h"
#include "webserver.h"
#include <cstdint>
#include <cstdlib>
#include "tasks.h"


Thread thread_main;
Thread thread_web_server;
Thread thread_jukebox;
Thread thread_stats;
Thread thread_sd;

UI::Display *display;

void webserver_task(WebServer *webServer) {
  while (true) {
    webServer->tick();
  }
}

Tasks::Stats *task_stats;

void stats_task() {
  task_stats->setDisplay(display);
  task_stats->start();
}

void jukebox_task() {
  Tasks::Jukebox *jukebox;
  jukebox->start();
}

int main(void) {
  display->init();

  thread_stats.start(stats_task);
  thread_jukebox.start(jukebox_task);

  Database db;
  WebServer webServer = WebServer(&db);

  display->log((uint8_t *)"[webserver]: starting");

  int status = webServer.start();
  if (status == 0) {
    display->log((uint8_t *)"[webserver]: error - No network interface found");
    return 1;
  }

  display->log((uint8_t *)"[webserver]: IP: 192.168.1.100");

  display->log((uint8_t *)"[host]: spawning webserver_thread");
  thread_web_server.start(callback(webserver_task, &webServer));

  Tasks::Storage *storage;
  storage->example(display);

  return 0;
}
