#ifndef __IO_SOCKET_H__
#define __IO_SOCKET_H__

#include "io_buffer.h"
#include "st.h"

#define SOCKET_READ_SIZE 4096

class IO_Socket
{
public:
    IO_Socket(st_netfd_t stfd_client, IO_Buffer *io_buffer);
    ~IO_Socket();

public:
    int read(ssize_t *nread);
    int read_nbytes(size_t nbytes, ssize_t *nread);
    int read_nbytes_av(size_t nbytes, ssize_t *nread);
    int read_nbytes_cycle(size_t nbytes, ssize_t *nread);
    int read_nbytes_cycle_av(size_t nbytes, ssize_t *nread);
    int write(char *buffer, size_t nbyte, ssize_t *nwrite);
    int writev(const struct iovec *iov, int iov_size, ssize_t *nwrite);

private:
    st_netfd_t stfd_client;
    IO_Buffer *io_buffer;
    st_utime_t timeout;
};

#endif