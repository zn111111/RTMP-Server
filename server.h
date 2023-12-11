#ifndef __SERVER_H__
#define __SERVER_H__

#include <unordered_map>
#include "st.h"
#include "io_buffer.h"

class Client;

class Server
{
public:
    Server();
    virtual ~Server();

public:
    int initialize();

    int listen(int port);

    int cycle();

private:
    void accept();

private:
    static void *listend_thread(void *arg);

private:
    st_netfd_t stfd;
    std::unordered_map<st_netfd_t, Client *> clients;
};

#endif
