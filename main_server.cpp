#include "server.h"

int main()
{
    int port = 1935;

    Server server;

    server.initialize();

    server.listen(port);
    server.cycle();
    return 0;
}