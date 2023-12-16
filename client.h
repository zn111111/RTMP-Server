#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <vector>
#include <string>
#include "server.h"
#include "io_socket.h"
#include "rtmp_protocol.h"
#include "io_message.h"
#include <unordered_map>

class Client
{
public:
    Client(st_netfd_t stfd_client);
    virtual ~Client();

public:
    void cycle();

private:
    static void *process_thread(void *arg);

public:
    int chunk_size;

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    RTMP_Protocol *rtmp;
    IO_Buffer *io_buffer;
};

#endif