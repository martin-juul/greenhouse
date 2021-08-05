#include "TCPSocket.h"

static const int MAX_CONN = 5;

class WebServer {
    public:
        int start();
        void tick();
        TCPSocket* getSocket();
};