#include "mbed.h"
#include "webserver.h"

int main(void) {
  WebServer webServer;

  webServer.start();

  // listening for http GET request
  while (true) {
    webServer.tick();
  }

  webServer.getSocket()->close();
  printf("Client socket closed\r\n");
}