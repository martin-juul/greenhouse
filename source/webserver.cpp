#include "mbed.h"
#include "webserver.h"
#include "EthernetInterface.h"
#include "TCPSocket.h"
#include "website.h"

#define IP "192.168.1.100"
#define GATEWAY "192.168.1.1"
#define NETMASK "255.255.255.0"
#define PORT 80

EthernetInterface *net;

TCPSocket server;
TCPSocket *client_socket;
SocketAddress client_address;
char rx_buffer[1024] = {0};
char tx_buffer[1024] = {0};

int requests = 0;

int WebServer::start() {
  printf("Starting\n");

  net = new EthernetInterface;

  if (!net) {
    return 0;
  }

  net->set_network(IP, NETMASK, GATEWAY); // include to use static IP address
  nsapi_size_or_error_t r = net->connect();
  if (r != 0) {
    printf("Error! net->connect() returned: %d\n", r);
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

  printf("IP address: %s\r\n", ip_addr ? ip_addr : "None");
  printf("Netmask: %s\r\n", netmask_addr ? netmask_addr : "None");
  printf("Gateway: %s\r\n\r\n", gateway_addr ? gateway_addr : "None");

  server.open(net);
  server.bind(ip);
  server.listen(MAX_CONN);

  return 1;
};

void WebServer::tick() {
  printf("=========================================\r\n");

  nsapi_error_t error = 0;

  client_socket = server.accept(&error);
  requests++;
  if (error != 0) {
    printf("Connection failed!\r\n");
  } else {
    client_socket->set_timeout(200);
    client_socket->getpeername(&client_address);
    printf("Client with IP address %s connected.\r\n\r\n",
           client_address.get_ip_address());
    error = client_socket->recv(rx_buffer, sizeof(rx_buffer));

    switch (error) {
    case 0:
      printf("Recieved buffer is empty.\r\n");
      break;

    case -1:
      printf("Failed to read data from client.\r\n");
      break;

    default:
      printf("Recieved Data: %d\n\r\n\r%.*s\r\n\n\r", strlen(rx_buffer), strlen(rx_buffer), rx_buffer);
      if (rx_buffer[0] == 'G' && rx_buffer[1] == 'E' && rx_buffer[2] == 'T') {
        // setup http response header & data
        sprintf(tx_buffer,
                "HTTP/1.1 200 OK\nContent-Length: %d\r\nContent-Type: "
                "text\r\nConnection: Close\r\n\r\n",
                strlen(rx_buffer));

        strcpy(tx_buffer, homepage);

        client_socket->send(tx_buffer, strlen(tx_buffer));
      }
      break;
    }
  }

  client_socket->close();
}

TCPSocket* WebServer::getSocket() {
    return client_socket;
}