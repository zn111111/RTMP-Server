#ifndef __RTMP_PROTOCOL_H__
#define __RTMP_PROTOCOL_H__

#include "io_socket.h"
#include "io_message.h"

class RTMP_Protocol
{
public:
    RTMP_Protocol(st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer);
    ~RTMP_Protocol();

public:
    int handshake();
    int recv_connect_message(IO_Message *io_message, std::unordered_map<int, std::vector<int>> &received_message_length_buffer);
    

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;
};

#endif