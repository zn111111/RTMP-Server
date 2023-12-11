#ifndef __ALL_KINDS_OF_MSEEAGES_H__
#define __ALL_KINDS_OF_MSEEAGES_H__

#include <unordered_map>
#include <vector>
#include "io_message.h"
#include "io_buffer.h"
#include "value_object.h"
#include "socket_demo_define.h"

class Set_Window_Size : public IO_Message
{
public:
    Set_Window_Size(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, int32_t window_size);
    ~Set_Window_Size();

    virtual int write_payload();

private:
    IO_Buffer *io_buffer;
    int32_t window_size;
};

class Set_Peer_Bandwidth : public IO_Message
{
public:
    Set_Peer_Bandwidth(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, int32_t bandwidth, char limit_type);
    ~Set_Peer_Bandwidth();

    virtual int write_payload();

private:
    IO_Buffer *io_buffer;
    int32_t bandwidth;
    char limit_type;
};

class Send_Object_Message : public IO_Message
{
public:
    Send_Object_Message(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, std::string command, double transaction_id, MESSAGE_OBJECT_TYPE type);
    ~Send_Object_Message();

    virtual int write_payload();
    int get_payload_size() const;

private:
    void init();

private:
    IO_Buffer *io_buffer;
    std::string command;
    double transaction_id;
    MESSAGE_OBJECT_TYPE type;
    std::vector<std::vector<std::pair<std::string, Value_Object *>>> vec;
};

class ReleaseStreamResponse : public IO_Message
{
public:
    ReleaseStreamResponse(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, double transaction_id);
    ~ReleaseStreamResponse();

    virtual int write_payload();

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;

    double transaction_id;
};

class SetChunkSize : public IO_Message
{
public:
    SetChunkSize(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, int chunk_size);
    ~SetChunkSize();

    virtual int write_payload();

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;

    int chunk_size;
};

class FCPublishResponse : public IO_Message
{
public:
    FCPublishResponse(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, double transaction_id);
    ~FCPublishResponse();

    virtual int write_payload();

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;

    double transaction_id;
};

class CreateStreamResponse : public IO_Message
{
public:
    CreateStreamResponse(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, double transaction_id, double stream_id);
    ~CreateStreamResponse();

    virtual int write_payload();

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;

    double transaction_id;
    double stream_id;
};

class ConnectMessage : public IO_Message
{
public:
    ConnectMessage(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer);
    ~ConnectMessage();

    virtual int read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer); 

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;
};

class ReleaseStream : public IO_Message
{
public:
    ReleaseStream(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer);
    ~ReleaseStream();

    virtual int read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer); 

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;
};

class FCPublish : public IO_Message
{
public:
    FCPublish(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer);
    ~FCPublish();

    virtual int read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer); 

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;
};

class CreateStream : public IO_Message
{
public:
    CreateStream(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer);
    ~CreateStream();

    virtual int read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer); 

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;
};

class CheckBW : public IO_Message
{
public:
    CheckBW(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer);
    ~CheckBW();

    virtual int read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer); 

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;
};

class UnknownMessage : public IO_Message
{
public:
    UnknownMessage(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer);
    ~UnknownMessage();

    virtual int read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer); 

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;
};

class Publish : public IO_Message
{
public:
    Publish(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer);
    ~Publish();

    virtual int read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer); 

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;
};

class OnFCPublish : public IO_Message
{
public:
    OnFCPublish(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, double transaction_id);
    ~OnFCPublish();

    virtual int write_payload();
    int get_payload_size();

private:
    void init();

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;

    double transaction_id;
    std::vector<std::vector<std::pair<std::string, Value_Object *>>> vec;
};

class OnStatus : public IO_Message
{
public:
    OnStatus(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, double transaction_id);
    ~OnStatus();

    virtual int write_payload();
    int get_payload_size();

private:
    void init();

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;

    double transaction_id;
    std::vector<std::vector<std::pair<std::string, Value_Object *>>> vec;
};

class SetDataFrame : public IO_Message
{
public:
    SetDataFrame(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer);
    ~SetDataFrame();

    virtual int read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer); 

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;
};

class IdentifyClient : public IO_Message
{
public:
    IdentifyClient(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer);
    ~IdentifyClient();

    int read_header_get_payloadLength(std::unordered_map<int, std::vector<int>> &received_message_length_buffer);

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;
};

class GetStreamLength : public IO_Message
{
public:
    GetStreamLength(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer);
    ~GetStreamLength();

    virtual int read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer); 

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;
};

class PlayCommandMessage : public IO_Message
{
public:
    PlayCommandMessage(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer);
    ~PlayCommandMessage();

    virtual int read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer); 

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;
};

class SrcPCUCStreamBegin : public IO_Message
{
public:
    SrcPCUCStreamBegin(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer);
    ~SrcPCUCStreamBegin();

    virtual int write_payload(); 

private:
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;
};

class PullStreamOnStatus : public IO_Message
{
public:
    PullStreamOnStatus(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, std::string command, double transaction_id, MESSAGE_OBJECT_TYPE type);
    ~PullStreamOnStatus();

    virtual int write_payload();

private:
    void init();

private:
    IO_Buffer *io_buffer;
    std::string command;
    double transaction_id;
    MESSAGE_OBJECT_TYPE type;
    std::vector<std::vector<std::pair<std::string, Value_Object *>>> vec;
};

class RtmpSampleAccess : public IO_Message
{
public:
    RtmpSampleAccess(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, std::string command);
    ~RtmpSampleAccess();

    virtual int write_payload();

private:
    IO_Buffer *io_buffer;
    std::string command;
};

class PullStreamOnStatus3 : public IO_Message
{
public:
    PullStreamOnStatus3(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, std::string command);
    ~PullStreamOnStatus3();

    virtual int write_payload();

private:
    IO_Buffer *io_buffer;
    std::string command;
};

class OnMetaData : public IO_Message
{
public:
    OnMetaData(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, std::string command);
    ~OnMetaData();

    virtual int write_payload();

private:
    IO_Buffer *io_buffer;
    std::string command;
};

#endif