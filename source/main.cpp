#include "database.h"
#include "mbed.h"
#include "mbed_wait_api.h"
#include "ui.h"
#include "webserver.h"
#include <cstdlib>

Thread thread_web_server;

void webserver_task(WebServer *webServer) {
  while (true) {
    webServer->tick();
  }
}

int main(void) {
  UI ui;
  ui.init();

  Database db;
  WebServer webServer = WebServer(&db);

  printf("[webserver]: starting\n");
  ui.log((uint8_t *)"[webserver]: starting");

  int status = webServer.start();
  if (status == 0) {
    printf("[webserver]: error - No network interface found\n");
    ui.log((uint8_t *)"[webserver]: error - No network interface found");
    exit(1);
  }

  printf("[webserver]: started successfully\n");
  ui.log((uint8_t *)"[webserver]: started successfully");

  printf("[host]: spawispawningng webserver_thread\n");
  ui.log((uint8_t *)"[host]: spawning webserver_thread");
  thread_web_server.start(callback(webserver_task, &webServer));

  while (true) {
  }

  webServer.getSocket()->close();
  printf("[webserver]: client socket closed\n");
  ui.log((uint8_t *)"[webserver]: client socket closed");
}