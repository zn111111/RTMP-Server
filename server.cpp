#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <st.h>
#include "server.h"
#include "client.h"

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
        printf("st initialize failed\n");
        return -1;
    }
    printf("st initialize success\n");

    return ret;
}

int Server::listen(int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (fd == -1)
    {
        printf("create linux socket error\n");
        return -1;
    }
    printf("create linux socket success\n");

    int reuse_socket = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_socket, sizeof(int)) == -1)
    {
        printf("setsockopt failed\n");
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
        printf("bind socket error\n");
        return -1;
    }
    printf("bind socket success\n");
    if (::listen(fd, 10) == -1)
    {
        printf("listen socket error\n");
        return -1;
    }
    printf("listen socket success\n");

    if ((stfd = st_netfd_open_socket(fd)) == NULL)
    {
        printf("st_netfd_open_socket failed\n");
        return -1;
    }

    if (st_thread_create(listend_thread, this, 0, 0) == NULL)
    {
        printf("st thread create error\n");
        return -1;
    }
    printf("st thread create success\n");
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
            printf("accept client error\n");
            continue;
        }
        printf("accept client success\n");

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