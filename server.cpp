#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <st.h>
#include "server.h"
#include "client.h"
#include "log.h"

Server::Server() {}
Server::~Server()
{
    std::unordered_map<st_netfd_t, Client *>::iterator iter;
    for (iter = clients.begin(); iter != clients.end(); ++iter)
    {
        if (iter->second)
        {
            delete iter->second;
        }          
    }
}

int Server::initialize()
{
    int ret = 0;
    if (st_init() == -1)
    {
        LOG_ERROR("st initialize failed");
        return -1;
    }
    LOG_INFO("st initialize success");

    return ret;
}

int Server::listen(int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (fd == -1)
    {
        LOG_ERROR("create linux socket error");
        return -1;
    }
    LOG_INFO("create linux socket success");

    int reuse_socket = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_socket, sizeof(int)) == -1)
    {
        LOG_ERROR("setsockopt failed");
        return -1;
    }

    // int flag = fcntl(fd, F_GETFL, 0);
    // flag |= O_NONBLOCK;
    // fcntl(fd, F_SETFL, flag);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(fd, (const sockaddr *)&addr, sizeof(addr)) == -1)
    {
        LOG_ERROR("bind socket error");
        return -1;
    }
    LOG_INFO("bind socket success");
    if (::listen(fd, 10) == -1)
    {
        LOG_ERROR("listen socket error");
        return -1;
    }
    LOG_INFO("listen socket success");

    if ((stfd = st_netfd_open_socket(fd)) == NULL)
    {
        LOG_ERROR("st_netfd_open_socket failed");
        return -1;
    }

    if (st_thread_create(listend_thread, this, 0, 0) == NULL)
    {
        LOG_ERROR("st thread create error");
        return -1;
    }
    LOG_INFO("st thread create success");
    return 0;
}

int Server::cycle()
{
    st_thread_exit(NULL);
    return 0;
}

void Server::accept()
{
    while (true)
    {
        sockaddr_in client_addr;
        st_netfd_t client_stfd = st_accept(stfd, NULL, NULL, ST_UTIME_NO_TIMEOUT);
        if (NULL == client_stfd)
        {
            LOG_ERROR("accept client error");
            continue;
        }
        LOG_INFO("accept client success");

        Client *client = new Client(client_stfd);
        clients.insert(std::pair<st_netfd_t, Client *>(client_stfd, client));

        client->cycle();
        st_usleep(0);
    }
}

void *Server::listend_thread(void *arg)
{
    if (arg)
    {
        Server *server = (Server *)arg;
        server->accept();      
    }
    return NULL;
}