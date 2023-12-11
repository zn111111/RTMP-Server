#include <cstdio>
#include "rtmp_protocol.h"
#include <vector>

RTMP_Protocol::RTMP_Protocol(st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer)
: stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer) {}

RTMP_Protocol::~RTMP_Protocol() {}

int RTMP_Protocol::handshake()
{
    ssize_t nread = 0;
    size_t nbytes = 1537; 
    if (io_socket->read_nbytes(nbytes, &nread) == -1)
    {
        return -1;
    }
    const char *p = io_buffer->get_data();
    if (*p != 0x03)
    {
        return -1;
    }

    char s0s1s2[3073] = {0};
    s0s1s2[0] = 0x03;

    ssize_t nwrite = 0;
    if (io_socket->write(s0s1s2, 3073, &nwrite) == -1)
    {
        return -1;
    }

    nbytes += 1536;
    if (io_socket->read_nbytes(nbytes, &nread) == -1)
    {
        return -1;
    }

    io_buffer->clear();

    return 0;
}

int RTMP_Protocol::recv_connect_message(IO_Message *io_message, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    while (true)
    {
        if (io_message->recv_message(received_message_length_buffer) == -1)
        {
            return -1;
        }

        if (io_message->get_command() == "connect")
        {
            break;
        }
    }

    io_buffer->clear();

    return 0;
}