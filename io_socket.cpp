#include <string.h>
#include "io_socket.h"
#include "util.h"
#include "io_message.h"
#include "log.h"

IO_Socket::IO_Socket(st_netfd_t stfd_client, IO_Buffer *io_buffer)
: stfd_client(stfd_client), io_buffer(io_buffer), timeout(5000000)
{}

IO_Socket::~IO_Socket() {}

int IO_Socket::read(ssize_t *nread)
{
    char buffer[SOCKET_READ_SIZE] = {0};
    *nread = st_read(stfd_client, buffer, SOCKET_READ_SIZE, timeout);
    if (*nread <= 0)
    {
        return -1;
    }
    io_buffer->push_back(buffer, (size_t)nread);

    return 0;
}

int IO_Socket::read_nbytes(size_t nbytes, ssize_t *nread)
{
    if (nbytes > io_buffer->size())
    {
        nbytes -= io_buffer->size();
        char buffer[nbytes];
        *nread = st_read_fully(stfd_client, buffer, nbytes, timeout);
        if (*nread != (ssize_t)nbytes)
        {
            return -1;
        }
        io_buffer->push_back(buffer, (size_t)*nread);
    }
    return 0;
}

int IO_Socket::read_nbytes_av(size_t nbytes, ssize_t *nread)
{
    if (nbytes > audio_video_buffer.size())
    {
        nbytes -= audio_video_buffer.size();
        char buffer[nbytes];
        *nread = st_read_fully(stfd_client, buffer, nbytes, timeout);
        if (*nread != (ssize_t)nbytes)
        {
            return -1;
        }
        audio_video_buffer.append(buffer, (size_t)*nread);
    }
    return 0;    
}

int IO_Socket::read_nbytes_cycle(size_t nbytes, ssize_t *nread)
{
    if (nbytes > io_buffer->size())
    {
        *nread = 0;
        nbytes -= io_buffer->size();
        char buffer[nbytes];
        char *p = buffer;
        while (*nread < nbytes)
        {
            ssize_t n_read = st_read_fully(stfd_client, p, nbytes - *nread, timeout);
            if (n_read < 0)
            {
                return -1;
            }
            else if ((*nread + n_read) < nbytes)
            {
                p += n_read;
                *nread += n_read;
                continue;
            }
            else
            {
                *nread += n_read;
                break;
            }
        }
        io_buffer->push_back(buffer, (size_t)*nread);
    }

    return 0;
}

int IO_Socket::read_nbytes_cycle_av(size_t nbytes, ssize_t *nread)
{
    if (nbytes > audio_video_buffer.size())
    {
        *nread = 0;
        nbytes -= audio_video_buffer.size();
        char buffer[nbytes];
        char *p = buffer;
        while (*nread < nbytes)
        {
            ssize_t n_read = st_read_fully(stfd_client, p, nbytes - *nread, timeout);
            if (n_read < 0)
            {
                return -1;
            }
            else if ((*nread + n_read) < nbytes)
            {
                p += n_read;
                *nread += n_read;
                continue;
            }
            else
            {
                *nread += n_read;
                break;
            }
        }
        audio_video_buffer.append(buffer, (size_t)*nread);
    }

    return 0;
}

int IO_Socket::write(char *buffer, size_t nbyte, ssize_t *nwrite)
{
    *nwrite = st_write(stfd_client, buffer, nbyte, timeout);
    if (*nwrite != (ssize_t)nbyte)
    {
        return -1;
    }

    return 0;
}

int IO_Socket::writev(const struct iovec *iov, int iov_size, ssize_t *nwrite)
{
    if (!iov)
    {
        return -1;
    }

    *nwrite = st_writev(stfd_client, iov, iov_size, timeout);
    if (*nwrite == -1)
    {
        LOG_ERROR("send failed, errno = %d", errno);
        return -1;
    }
    return 0;
}