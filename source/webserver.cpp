#include "http/http_response_builder.h"
#include "webserver.h"
#include "EthernetInterface.h"
#include "mbed.h"
#include "website.h"
#include <cstring>
#include <string>

#define IP "192.168.1.100"
#define GATEWAY "192.168.1.1"
#define NETMASK "255.255.255.0"
#define PORT 80

Database *db;

EthernetInterface *net;

TCPSocket server;
TCPSocket *client_socket;
SocketAddress client_address;
char rx_buffer[1024] = {0};
char tx_buffer[1024] = {0};

int requests = 0;

WebServer::WebServer(Database *database) { db = database; }

int WebServer::start() {
  net = new EthernetInterface;

  if (!net) {
    return 0;
  }

  net->set_network(IP, NETMASK, GATEWAY); // include to use static IP address
  nsapi_size_or_error_t r = net->connect();
  if (r != 0) {
    printf("[webserver]: error! net->connect() returned: %d\n", r);
    return r;
  }

  // Show the network address
  SocketAddress ip;
  SocketAddress netmask;
  SocketAddress gateway;

  net->get_ip_address(&ip);
  net->get_netmask(&netmask);
  net->get_gateway(&gateway);

  ip.set_port(PORT);

  const char *ip_addr = ip.get_ip_address();
  const char *netmask_addr = netmask.get_ip_address();
  const char *gateway_addr = gateway.get_ip_address();

  printf("[webserver]: IP address: %s\n", ip_addr ? ip_addr : "None");
  printf("[webserver]: Netmask: %s\n", netmask_addr ? netmask_addr : "None");
  printf("[webserver]: Gateway: %s\n", gateway_addr ? gateway_addr : "None");

  server.open(net);
  server.bind(ip);
  server.listen(MAX_CONN);

  return 1;
};

void http_ok() {
  sprintf(tx_buffer,
          "HTTP/1.1 200 OK\n"
          "Content-Length: %d\n"
          "Content-Type: text\n"
          "Connection: Close\n",
          strlen(rx_buffer));
}

void http_no_content() {
  sprintf(tx_buffer, "HTTP/1.1 204 No Content\n"
                     "Connection: Close\n");
}

void http_not_found() {
  sprintf(tx_buffer, "HTTP/1.1 404 Not Found\n"
                     "Connection: Close\n");
}

void http_internal_server_error() {
  sprintf(tx_buffer, "HTTP/1.1 500 Internal Server Error\n"
                     "Connection: Close\n");
}

void WebServer::tick() {
  printf("=========================================\n");

  nsapi_error_t error = 0;

  client_socket = server.accept(&error);
  requests++;
  if (error != 0) {
    printf("[webserver]: Connection failed!\n");
  } else {
    client_socket->set_timeout(
        0); // timeout of 0 makes it a non-blocking connection
    client_socket->getpeername(&client_address);
    printf("[webserver]: Client with IP address %s connected.\n",
           client_address.get_ip_address());
    error = client_socket->recv(rx_buffer, sizeof(rx_buffer));

    switch (error) {
    case 0:
      printf("[webserver]: Recieved buffer is empty.\n");
      http_internal_server_error();
      break;

    case -1:
      printf("[webserver]: Failed to read data from client.\n");
      http_internal_server_error();
      break;

    default:
      printf("[webserver]: Recieved Data: %d bytes\n%.*s\n", strlen(rx_buffer),
             strlen(rx_buffer), rx_buffer);

      printf("%c", rx_buffer[4]);
      if (rx_buffer[0] == 'G' && rx_buffer[1] == 'E' && rx_buffer[2] == 'T' &&
          rx_buffer[4] == '/' && rx_buffer[5] == ' ') {
        // setup http response header & data
        http_ok();

        strcpy(tx_buffer, homepage);
      } else if (rx_buffer[0] == 'P' && rx_buffer[1] == 'O' &&
                 rx_buffer[2] == 'S' && rx_buffer[3] == 'T') {
        // POST request
        string s = string(rx_buffer);
        int len = s.length();
        string data = s.substr(len - 11, len);

        string temp = data.substr(0, 4);
        string dewity = data.substr(4, 4).substr(1, 2); // wat 🤡
        string humidity = data.substr(8, 9);

        Row r = Row();
        r.temperature = temp;
        r.dewity = dewity;
        r.humidity = humidity;

        db->append(r);

        HttpResponseBuilder builder(204);
        builder.send(client_socket, nullptr, 0);

        http_no_content();
      } else {
        http_not_found();
      }

      client_socket->send(tx_buffer, strlen(tx_buffer));
      break;
    }
  }

  client_socket->close();
}

TCPSocket *WebServer::getSocket() { return client_socket; }