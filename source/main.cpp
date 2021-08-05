#include "mbed.h"
#include "webserver.h"
#include <cstdlib>

int main(void) {
  WebServer webServer;

  int status = webServer.start();
  if (status == 0) {
    printf("Error: No network interface found.\n");
    exit(1);
  }

  // listening for http GET request
  while (true) {
    webServer.tick();
  }

  webServer.getSocket()->close();
  printf("Client socket closed\r\n");
}