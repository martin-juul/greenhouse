#pragma once

#include "TCPSocket.h"
#include "database.h"
#include "socket.h"

static const int MAX_CONN = 5;

class WebServer {
public:
  WebServer(Database *database);

  /** Starts the webserver
   *
   * @retval void
   */
  int start();
  /** One "revolution" of parsing incoming/outgoing connections
   *
   * @retval void
   */
  void tick();
  /** Get the underlying socket.
   *
   * Use this socket to close the server.
   *
   * @retval TCPSocket
   */
  TCPSocket *getSocket();
};