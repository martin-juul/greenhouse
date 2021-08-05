#include "mbed.h"
#include "webserver.h"

WebServer *server;

int main(void) {
  server = new WebServer;
  server->start();

  // listening for http GET request
  while (true) {
    server->tick();
  }

  server->getSocket()->close();
  printf("Client socket closed\r\n");
}