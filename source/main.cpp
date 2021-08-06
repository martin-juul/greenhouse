#include "database.h"
#include "mbed.h"
#include "mbed_wait_api.h"
#include "webserver.h"
#include <cstdlib>

Thread thread_web_server;
Database database;

void webserver_task(WebServer* webServer) {
  while (true) {
    webServer->tick();
  }
}

int main(void) {
  Database db;
  WebServer webServer = WebServer(&database);

  printf("[webserver]: starting\n");
  
  int status = webServer.start();
  if (status == 0) {
    printf("Error: No network interface found.\n");
    exit(1);
  }

  printf("[webserver]: started successfully\n");

  printf("[host]: spawing webserver_thread\n");
  thread_web_server.start(callback(webserver_task, &webServer));

  // listening for http GET request
  while (true) {
    
  }

  webServer.getSocket()->close();
  printf("Client socket closed\r\n");
}