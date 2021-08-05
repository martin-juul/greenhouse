#include "mbed.h"
#include "mbed_wait_api.h"
#include "webserver.h"
#include <cstdlib>

Thread thread;
WebServer webServer;

void webserver_thread() {
  while (true) {
    webServer.tick();
  }
}

int main(void) {
  int status = webServer.start();
  if (status == 0) {
    printf("Error: No network interface found.\n");
    exit(1);
  }

  thread.start(webserver_thread);

  // listening for http GET request
  while (true) {
    
  }

  webServer.getSocket()->close();
  printf("Client socket closed\r\n");
}