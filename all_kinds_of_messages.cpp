#include "all_kinds_of_messages.h"
#include "util.h"
#include "log.h"
#include <string.h>

Set_Window_Size::Set_Window_Size(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, int32_t window_size)
: IO_Message(client, stfd_client, io_socket, io_buffer), io_buffer(io_buffer), window_size(window_size) {}

Set_Window_Size::~Set_Window_Size() {}

int Set_Window_Size::write_payload()
{
    char buffer[4] = {0};
    if (se2be_4bytes((char *)&window_size, buffer) == -1)
    {
        return -1;
    }
    io_buffer->push_back(buffer, 4);
    return 0;
}

Set_Peer_Bandwidth::Set_Peer_Bandwidth(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, int32_t bandwidth, char limit_type)
: IO_Message(client, stfd_client, io_socket, io_buffer), io_buffer(io_buffer), bandwidth(bandwidth), limit_type(limit_type) {}

Set_Peer_Bandwidth::~Set_Peer_Bandwidth() {}

int Set_Peer_Bandwidth::write_payload()
{
    char dst[4] = {0};
    if (se2be_4bytes((char *)&bandwidth, dst) == -1)
    {
        return -1;
    }
    io_buffer->push_back(dst, 4);
    io_buffer->push_back(&limit_type, 1);
    return 0;
}

Send_Object_Message::Send_Object_Message(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, std::string command, double transaction_id, MESSAGE_OBJECT_TYPE type)
: IO_Message(client, stfd_client, io_socket, io_buffer), io_buffer(io_buffer), command(command), transaction_id(transaction_id), type(type)
{
    init();
}

Send_Object_Message::~Send_Object_Message()
{
    for (int i = 0; i < vec.size(); i++)
    {
        for (const auto &it : vec[i])
        {
            if (it.second)
            {
                delete it.second;
            }
        }
    }
}

int Send_Object_Message::write_payload()
{
    char marker_str = 0x02;
    io_buffer->push_back(&marker_str, 1);
    int16_t len = command.size();
    char buffer_len[2] = {0};
    if (se2be_2bytes((char *)&len, buffer_len) == -1)
    {
        return -1;
    }
    io_buffer->push_back(buffer_len, 2);
    io_buffer->push_back(command.c_str(), len);

    char marker_num = 0x00;
    io_buffer->push_back(&marker_num, 1);
    int64_t buffer_id = 0;
    memcpy(&buffer_id, &transaction_id, sizeof(transaction_id));
    char dst[8] = {0};
    if (se2be_8bytes((char *)&buffer_id, dst) == -1)
    {
        return -1;
    }
    io_buffer->push_back(dst, 8);

    if (vec.size() == 0)
    {
        char marker_0x05 = 0x05;
        io_buffer->push_back(&marker_0x05, 1);
        return 0;
    }

    for (int i = 0; i < vec.size(); i++)
    {
        char marker_object = 0x03;
        io_buffer->push_back(&marker_object, 1);
        for (int j = 0; j < vec[i].size(); j++)
        {
            int16_t len_key = vec[i][j].first.size();
            char buffer_key_len[2] = {0};
            if (se2be_2bytes((char *)&len_key, buffer_key_len) == -1)
            {
                return -1;
            }
            io_buffer->push_back(buffer_key_len, 2);
            io_buffer->push_back(vec[i][j].first.c_str(), (size_t)len_key);
            
            if (dynamic_cast<Value_String *>(vec[i][j].second))
            {
                io_buffer->push_back(&marker_str, 1);
                Value_String *value_string = dynamic_cast<Value_String *>(vec[i][j].second);
                int16_t len_value_str = (int16_t)value_string->value.size();
                char buffer_len_value[2] = {0};
                if (se2be_2bytes((char *)&len_value_str, buffer_len_value) == -1)
                {
                    return -1;
                }
                io_buffer->push_back(buffer_len_value, 2);
                io_buffer->push_back(value_string->value.c_str(), (size_t)len_value_str);
            }
            else if (dynamic_cast<Value_Num *>(vec[i][j].second))
            {
                io_buffer->push_back(&marker_num, 1);
                Value_Num *value_num = dynamic_cast<Value_Num *>(vec[i][j].second);
                int64_t buffer_value_num = 0;
                memcpy(&buffer_value_num, &value_num->value, 8);
                char dst_buffer[8] = {0};
                if (se2be_8bytes((char *)&buffer_value_num, dst_buffer) == -1)
                {
                    return -1;
                }
                io_buffer->push_back(dst_buffer, 8);
            }
            else if (dynamic_cast<Value_Len *>(vec[i][j].second))
            {
                char marker_len = 0x08;
                io_buffer->push_back(&marker_len, 1);
                char buffer_0x08_value[4] = {0};
                Value_Len *value_len = dynamic_cast<Value_Len *>(vec[i][j].second);
                if (se2be_4bytes((char *)&value_len->value, buffer_0x08_value) == -1)
                {
                    return -1;
                }
                io_buffer->push_back(buffer_0x08_value, 4);
            }
            else
            {
                LOG_ERROR("write_payload failed, the value type does not match");
                return -1;
            }
        }
        int16_t flag_0x00 = 0x00;
        char buffer_flag_0x00[2] = {0};    
        if (se2be_2bytes((char *)&flag_0x00, buffer_flag_0x00) == -1)
        {
            return -1;
        }
        io_buffer->push_back(buffer_flag_0x00, 2);
        char marker_0x09 = 0x09;
        io_buffer->push_back(&marker_0x09, 1);
    }

    if (vec.size() > 1)
    {
        int16_t flag_0x00 = 0x00;
        char buffer_flag_0x00[2] = {0};
        if (se2be_2bytes((char *)&flag_0x00, buffer_flag_0x00) == -1)
        {
            return -1;
        }
        io_buffer->push_back(buffer_flag_0x00, 2);
        char marker_0x09 = 0x09;
        io_buffer->push_back(&marker_0x09, 1);
    }

    return 0;
}

int Send_Object_Message::get_payload_size() const
{
    int payload_size = 3 + command.size() + 9;
    if (type == ON_BW_DONE)
    {
        payload_size += 1;
        return payload_size;
    }

    for (int i = 0; i < vec.size(); i++)
    {
        payload_size += 1;
        for (int j = 0; j < vec[i].size(); j++)
        {
            payload_size += 2;
            payload_size += vec[i][j].first.size();
            payload_size += 1;
            if (dynamic_cast<Value_String *>(vec[i][j].second))
            {
                payload_size += 2;
                Value_String * value_str = dynamic_cast<Value_String *>(vec[i][j].second);
                payload_size += value_str->value.size();
            }
            else if (dynamic_cast<Value_Num *>(vec[i][j].second))
            {
                payload_size += 8;
            }
            else if (dynamic_cast<Value_Len *>(vec[i][j].second))
            {
                payload_size += 4;
            }
            else
            {
                LOG_ERROR("get_payload_size failed, the value type does not match");
                return -1;
            }
        }
        payload_size += 3;
    }
    if (vec.size() > 1)
    {
        payload_size += 3;
    }
    return payload_size;
}

void Send_Object_Message::init()
{
    if (type == ON_BW_DONE)
    {
        return;
    }

    if (type == PULL_ONSTATUS3)
    {
        std::vector<std::pair<std::string, Value_Object *>> vec1;
        const char *value = "NetStream.Data.Start";
        Value_Object *value_object = new Value_String(value, strlen(value));
        vec1.push_back(std::pair<std::string, Value_Object *>("code", value_object));
        
        vec.push_back(vec1);

        return;
    }

    if (type == CONNECT_APP_RESPONSE)
    {
        std::vector<std::pair<std::string, Value_Object *>> vec1;
        const char *value = "FMS/3,5,3,888";
        Value_Object *value_object = new Value_String(value, strlen(value));
        vec1.push_back(std::pair<std::string, Value_Object *>("fmsVer", value_object));
        value_object = new Value_Num(127);
        vec1.push_back(std::pair<std::string, Value_Object *>("capabilities", value_object));
        value_object = new Value_Num(1);
        vec1.push_back(std::pair<std::string, Value_Object *>("mode", value_object));
        vec.push_back(vec1);

        std::vector<std::pair<std::string, Value_Object *>> vec2;
        value = "status";
        value_object = new Value_String(value, strlen(value));
        vec2.push_back(std::pair<std::string, Value_Object *>("level", value_object));
        value = "NetConnection.Connect.Success";
        value_object = new Value_String(value, strlen(value));
        vec2.push_back(std::pair<std::string, Value_Object *>("code", value_object));
        value = "Connection succeeded";
        value_object = new Value_String(value, strlen(value));
        vec2.push_back(std::pair<std::string, Value_Object *>("description", value_object));
        value_object = new Value_Num(0);
        vec2.push_back(std::pair<std::string, Value_Object *>("objectEncoding", value_object));
        value_object = new Value_Len(2028717191);
        vec2.push_back(std::pair<std::string, Value_Object *>("data", value_object));
        value = "3,5,3,888";
        value_object = new Value_String(value, strlen(value));
        vec2.push_back(std::pair<std::string, Value_Object *>("version", value_object));
        value = "srs(simple rtmp server)";
        value_object = new Value_String(value, strlen(value));
        vec2.push_back(std::pair<std::string, Value_Object *>("server", value_object));
        value = "https://github.com/winlinvip/simple-rtmp-server";
        value_object = new Value_String(value, strlen(value));
        vec2.push_back(std::pair<std::string, Value_Object *>("srs_url", value_object));
        value = "0.1";
        value_object = new Value_String(value, strlen(value));
        vec2.push_back(std::pair<std::string, Value_Object *>("srs_version", value_object));
        vec.push_back(vec2);
    }
}

ReleaseStreamResponse::ReleaseStreamResponse(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, double transaction_id) : stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer), transaction_id(transaction_id),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

ReleaseStreamResponse::~ReleaseStreamResponse() {}

int ReleaseStreamResponse::write_payload()
{
    char marker_0x02 = 0x02;
    io_buffer->push_back(&marker_0x02, 1);
    int16_t src_len = 7;
    int16_t dst_len = 0;
    if (se2be_2bytes((char *)&src_len, (char *)&dst_len) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&dst_len, 2);
    const char *command = "_result";
    io_buffer->push_back(command, 7);
    char marker_0x00 = 0x00;
    io_buffer->push_back(&marker_0x00, 1);
    int64_t transaction_id_buffer = 0;
    memcpy(&transaction_id_buffer, &transaction_id, sizeof(transaction_id));
    int64_t dst_transaction_id = 0;
    if (se2be_8bytes((char *)&transaction_id_buffer, (char *)&dst_transaction_id) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&dst_transaction_id, 8);
    char marker_0x05 = 0x05;
    io_buffer->push_back(&marker_0x05, 1);
    char marker_0x06 = 0x06;
    io_buffer->push_back(&marker_0x06, 1);
    return 0;
}

SetChunkSize::SetChunkSize(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, int chunk_size) : stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer), chunk_size(chunk_size),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

SetChunkSize::~SetChunkSize() {}

int SetChunkSize::write_payload()
{
    int32_t chunk_size_buffer = 0;
    if (se2be_4bytes((char *)&chunk_size, (char *)&chunk_size_buffer) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&chunk_size_buffer, 4);
    return 0;
}

FCPublishResponse::FCPublishResponse(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, double transaction_id) : stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer), transaction_id(transaction_id),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

FCPublishResponse::~FCPublishResponse() {}

int FCPublishResponse::write_payload()
{
    char marker_0x02 = 0x02;
    io_buffer->push_back(&marker_0x02, 1);
    int16_t src_len = 7;
    int16_t dst_len = 0;
    if (se2be_2bytes((char *)&src_len, (char *)&dst_len) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&dst_len, 2);
    const char *command = "_result";
    io_buffer->push_back(command, 7);
    char marker_0x00 = 0x00;
    io_buffer->push_back(&marker_0x00, 1);
    int64_t transaction_id_buffer = 0;
    memcpy(&transaction_id_buffer, &transaction_id, sizeof(transaction_id));
    int64_t dst_transaction_id = 0;
    if (se2be_8bytes((char *)&transaction_id_buffer, (char *)&dst_transaction_id) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&dst_transaction_id, 8);
    char marker_0x05 = 0x05;
    io_buffer->push_back(&marker_0x05, 1);
    char marker_0x06 = 0x06;
    io_buffer->push_back(&marker_0x06, 1);
    return 0;
}

CreateStreamResponse::CreateStreamResponse(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, double transaction_id, double stream_id) : stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer), transaction_id(transaction_id), stream_id(stream_id),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

CreateStreamResponse::~CreateStreamResponse() {}
int CreateStreamResponse::write_payload()
{
    char marker_0x02 = 0x02;
    io_buffer->push_back(&marker_0x02, 1);
    int16_t src_len = 7;
    int16_t dst_len = 0;
    if (se2be_2bytes((char *)&src_len, (char *)&dst_len) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&dst_len, 2);
    const char *command = "_result";
    io_buffer->push_back(command, 7);
    char marker_0x00 = 0x00;
    io_buffer->push_back(&marker_0x00, 1);
    int64_t transaction_id_buffer = 0;
    memcpy(&transaction_id_buffer, &transaction_id, sizeof(transaction_id));
    int64_t dst_transaction_id = 0;
    if (se2be_8bytes((char *)&transaction_id_buffer, (char *)&dst_transaction_id) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&dst_transaction_id, 8);
    char marker_0x05 = 0x05;
    io_buffer->push_back(&marker_0x05, 1);
    char marker_0x002 = 0x00;
    io_buffer->push_back(&marker_0x002, 1);
    int64_t src_stream_id = 0;
    memcpy(&src_stream_id, &stream_id, 8);
    int64_t dst_stream_id = 0;
    if (se2be_8bytes((char *)&src_stream_id, (char *)&dst_stream_id) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&dst_stream_id, 8);
    return 0;
}

ConnectMessage::ConnectMessage(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer) : stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

ConnectMessage::~ConnectMessage() {}

int ConnectMessage::read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    //不知道为什么，客户端发送的connect命令，object部分tcUrl对应的value里多了个字节，因此实际的payload比message header里的message length标识的大小大1个字节

    if (message_length <= 0)
    {
        if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
        {
            return -1;
        }
        size_t v_size = received_message_length_buffer[csid].size();
        message_length = received_message_length_buffer[csid][v_size - 1] + 1;
    }
    else
    {
        message_length += 1;
    }

    size_t nbytes = bh_size + mh_size + message_length;
    ssize_t nread = 0;
    if (io_socket->read_nbytes_cycle(nbytes, &nread) == -1)
    {
        return -1;
    }
    //这里使用ffmpeg推流的时候，传输的数据rtmp://192.168.0.21:1935/live中
    //不知道为什么会多出一个字节c3（在wireshark中显示的是原始数据，也就是不考虑符号）
    //实际上查看二进制数据的补码知道该数据是负数，也就是-61，因此要过滤掉-61
    size_t error_pos = 0;
    if ((error_pos = io_buffer->find(-61, 0)) != std::string::npos)
    {
        io_buffer->erase(error_pos, 1);
    }

    const char *p = io_buffer->get_data() + bh_size + mh_size;

    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_STRING)
    {
        return -1;
    }

    int16_t len = 0;
    if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
    {
        return -1;
    }

    if (len <= 0)
    {
        return -1;
    }

    p += 2;
    std::string command_name;
    command_name.append(p, len);

    p += len;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_NUM)
    {
        return -1;
    }

    int64_t transaction_id = 0;
    if (p + 8 > io_buffer->get_data() + io_buffer->size() || be2se_8bytes(p, (char *)&transaction_id) == -1)
    {
        return -1;
    }
    
    p += 8;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_OBJECT)
    {
        return -1;
    }

    if (read_object(bh_size, mh_size, p) == -1)
    {
        return -1;
    }

    copy(command_name, transaction_id);

    return 0;
}

ReleaseStream::ReleaseStream(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer) : stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

ReleaseStream::~ReleaseStream() {}

int ReleaseStream::read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    if (message_length <= 0)
    {
        if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
        {
            return -1;
        }
        size_t v_size = received_message_length_buffer[csid].size();
        message_length = received_message_length_buffer[csid][v_size - 1];
    }

    size_t nbytes = bh_size + mh_size + message_length;
    ssize_t nread = 0;
    if (io_socket->read_nbytes_cycle(nbytes, &nread) == -1)
    {
        return -1;
    }
    const char *p = io_buffer->get_data() + bh_size + mh_size;

    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_STRING)
    {
        return -1;
    }

    int16_t len = 0;
    if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
    {
        return -1;
    }

    if (len <= 0)
    {
        return -1;
    }

    p += 2;
    std::string command_name;
    command_name.append(p, len);

    p += len;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_NUM)
    {
        return -1;
    }

    int64_t transaction_id = 0;
    if (p + 8 > io_buffer->get_data() + io_buffer->size() || be2se_8bytes(p, (char *)&transaction_id) == -1)
    {
        return -1;
    }
    
    p += 8;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAFE_NULL_OBJECT)
    {
        return -1;
    }

    copy(command_name, transaction_id);

    return 0;
}

FCPublish::FCPublish(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer) : stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

FCPublish::~FCPublish() {}

int FCPublish::read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    if (message_length <= 0)
    {
        if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
        {
            return -1;
        }
        size_t v_size = received_message_length_buffer[csid].size();
        message_length = received_message_length_buffer[csid][v_size - 1];
    }

    size_t nbytes = bh_size + mh_size + message_length;
    ssize_t nread = 0;
    if (io_socket->read_nbytes_cycle(nbytes, &nread) == -1)
    {
        return -1;
    }
    const char *p = io_buffer->get_data() + bh_size + mh_size;

    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_STRING)
    {
        return -1;
    }

    int16_t len = 0;
    if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
    {
        return -1;
    }

    if (len <= 0)
    {
        return -1;
    }

    p += 2;
    std::string command_name;
    command_name.append(p, len);

    p += len;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_NUM)
    {
        return -1;
    }

    int64_t transaction_id = 0;
    if (p + 8 > io_buffer->get_data() + io_buffer->size() || be2se_8bytes(p, (char *)&transaction_id) == -1)
    {
        return -1;
    }
    
    p += 8;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAFE_NULL_OBJECT)
    {
        return -1;
    }

    copy(command_name, transaction_id);

    return 0;
}

CreateStream::CreateStream(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer): stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

CreateStream::~CreateStream() {}

int CreateStream::read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    if (message_length <= 0)
    {
        if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
        {
            return -1;
        }
        size_t v_size = received_message_length_buffer[csid].size();
        message_length = received_message_length_buffer[csid][v_size - 1];
    }

    size_t nbytes = bh_size + mh_size + message_length;
    ssize_t nread = 0;
    if (io_socket->read_nbytes_cycle(nbytes, &nread) == -1)
    {
        return -1;
    }
    const char *p = io_buffer->get_data() + bh_size + mh_size;

    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_STRING)
    {
        return -1;
    }

    int16_t len = 0;
    if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
    {
        return -1;
    }

    if (len <= 0)
    {
        return -1;
    }

    p += 2;
    std::string command_name;
    command_name.append(p, len);

    p += len;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_NUM)
    {
        return -1;
    }

    int64_t transaction_id = 0;
    if (p + 8 > io_buffer->get_data() + io_buffer->size() || be2se_8bytes(p, (char *)&transaction_id) == -1)
    {
        return -1;
    }
    
    p += 8;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAFE_NULL_OBJECT)
    {
        return -1;
    }

    copy(command_name, transaction_id);

    return 0;
}

CheckBW::CheckBW(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer) : stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

CheckBW::~CheckBW() {}

int CheckBW::read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    if (message_length <= 0)
    {
        if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
        {
            return -1;
        }
        size_t v_size = received_message_length_buffer[csid].size();
        message_length = received_message_length_buffer[csid][v_size - 1];
    }

    size_t nbytes = bh_size + mh_size + message_length;
    ssize_t nread = 0;
    if (io_socket->read_nbytes_cycle(nbytes, &nread) == -1)
    {
        return -1;
    }

    LOG_INFO("drop _checkbw message");
    return 0;
}

UnknownMessage::UnknownMessage(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer): stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

UnknownMessage::~UnknownMessage() {}

int UnknownMessage::read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    if (message_length <= 0)
    {
        if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
        {
            return -1;
        }
        size_t v_size = received_message_length_buffer[csid].size();
        message_length = received_message_length_buffer[csid][v_size - 1];
    }

    size_t nbytes = bh_size + mh_size + message_length;
    ssize_t nread = 0;
    if (io_socket->read_nbytes_cycle(nbytes, &nread) == -1)
    {
        return -1;
    }

    return 0;
}

Publish::Publish(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer) : stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

Publish::~Publish() {}

int Publish::read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    if (message_length <= 0)
    {
        if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
        {
            return -1;
        }
        size_t v_size = received_message_length_buffer[csid].size();
        message_length = received_message_length_buffer[csid][v_size - 1];
    }

    size_t nbytes = bh_size + mh_size + message_length;
    ssize_t nread = 0;
    if (io_socket->read_nbytes_cycle(nbytes, &nread) == -1)
    {
        return -1;
    }
    const char *p = io_buffer->get_data() + bh_size + mh_size;

    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_STRING)
    {
        return -1;
    }

    int16_t len = 0;
    if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
    {
        return -1;
    }

    if (len <= 0)
    {
        return -1;
    }

    p += 2;
    std::string command_name;
    command_name.append(p, len);

    p += len;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_NUM)
    {
        return -1;
    }

    int64_t transaction_id = 0;
    if (p + 8 > io_buffer->get_data() + io_buffer->size() || be2se_8bytes(p, (char *)&transaction_id) == -1)
    {
        return -1;
    }
    
    p += 8;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAFE_NULL_OBJECT)
    {
        return -1;
    }

    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_STRING)
    {
        return -1;
    }

    if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
    {
        return -1;
    }

    if (len != 0)
    {
        return -1;
    }

    p += 2;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_STRING)
    {
        return -1;
    }

    if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
    {
        return -1;
    }

    if (len <= 0)
    {
        return -1;
    }
    p += 2;

    std::string pubuish_type;
    pubuish_type.append(p, len);

    LOG_INFO("command_name = %s, transaction_id = %ld, publish_type = %s\n", command_name.c_str(), transaction_id, pubuish_type.c_str());

    return 0;
}

OnFCPublish::OnFCPublish(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, double transaction_id): stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer), transaction_id(transaction_id),
IO_Message(client, stfd_client, io_socket, io_buffer) {init();}

OnFCPublish::~OnFCPublish()
{
    for (int i = 0; i < vec.size(); i++)
    {
        for (int j = 0; j < vec[i].size(); j++)
        {
            delete vec[i][j].second;
        }
    }
}

int OnFCPublish::write_payload()
{
    char marker_0x02 = 0x02;
    io_buffer->push_back(&marker_0x02, 1);
    int16_t src_len = 11;
    int16_t dst_len = 0;
    if (se2be_2bytes((char *)&src_len, (char *)&dst_len) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&dst_len, 2);
    const char *command = "onFCPublish";
    io_buffer->push_back(command, 11);
    char marker_0x00 = 0x00;
    io_buffer->push_back(&marker_0x00, 1);
    int64_t transaction_id_buffer = 0;
    memcpy(&transaction_id_buffer, &transaction_id, sizeof(transaction_id));
    int64_t dst_transaction_id = 0;
    if (se2be_8bytes((char *)&transaction_id_buffer, (char *)&dst_transaction_id) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&dst_transaction_id, 8);
    char marker_0x05 = 0x05;
    io_buffer->push_back(&marker_0x05, 1);

    if (write_object(vec) == -1)
    {
        return -1;
    }

    return 0;
}

int OnFCPublish::get_payload_size()
{
    return IO_Message::get_payload_size(vec);
}

void OnFCPublish::init()
{
   std::vector<std::pair<std::string, Value_Object *>> vp;
    char v_of_code[] = "NetStream.Publish.Start";
    Value_Object *value = new Value_String(v_of_code, strlen(v_of_code));
    vp.push_back(std::pair<std::string, Value_Object *>("code", value));

    char v_of_desc[] = "Started publishing stream.";
    value = new Value_String(v_of_desc, strlen(v_of_desc));
    vp.push_back(std::pair<std::string, Value_Object *>("description", value));

    vec.push_back(vp);
}

OnStatus::OnStatus(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, double transaction_id) : stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer), transaction_id(transaction_id),
IO_Message(client, stfd_client, io_socket, io_buffer) {init();}

OnStatus::~OnStatus()
{
    for (int i = 0; i < vec.size(); i++)
    {
        for (int j = 0; j < vec[i].size(); j++)
        {
            delete vec[i][j].second;
        }
    }
}

int OnStatus::write_payload()
{
    char marker_0x02 = 0x02;
    io_buffer->push_back(&marker_0x02, 1);
    int16_t src_len = 8;
    int16_t dst_len = 0;
    if (se2be_2bytes((char *)&src_len, (char *)&dst_len) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&dst_len, 2);
    const char *command = "onStatus";
    io_buffer->push_back(command, 8);
    char marker_0x00 = 0x00;
    io_buffer->push_back(&marker_0x00, 1);
    int64_t transaction_id_buffer = 0;
    memcpy(&transaction_id_buffer, &transaction_id, sizeof(transaction_id));
    int64_t dst_transaction_id = 0;
    if (se2be_8bytes((char *)&transaction_id_buffer, (char *)&dst_transaction_id) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&dst_transaction_id, 8);
    char marker_0x05 = 0x05;
    io_buffer->push_back(&marker_0x05, 1);

    if (write_object(vec) == -1)
    {
        return -1;
    }

    return 0;
}
int OnStatus::get_payload_size()
{
    return IO_Message::get_payload_size(vec);
}

void OnStatus::init()
{
    std::vector<std::pair<std::string, Value_Object *>> vp;
    char v_of_level[] = "status";
    Value_Object *value = new Value_String(v_of_level, strlen(v_of_level));
    vp.push_back(std::pair<std::string, Value_Object *>("level", value));

    char v_of_code[] = "NetStream.Publish.Start";
    value = new Value_String(v_of_code, strlen(v_of_code));
    vp.push_back(std::pair<std::string, Value_Object *>("code", value));

    char v_of_desc[] = "Started publishing stream.";
    value = new Value_String(v_of_desc, strlen(v_of_desc));
    vp.push_back(std::pair<std::string, Value_Object *>("description", value));

    char v_of_cid[] = "ASAICiss";
    value = new Value_String(v_of_cid, strlen(v_of_cid));
    vp.push_back(std::pair<std::string, Value_Object *>("clientid", value));

    vec.push_back(vp);
}

SetDataFrame::SetDataFrame(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer): stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

SetDataFrame::~SetDataFrame() {}

int SetDataFrame::read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    if (message_length <= 0)
    {
        if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
        {
            return -1;
        }
        size_t v_size = received_message_length_buffer[csid].size();
        message_length = received_message_length_buffer[csid][v_size - 1];
    }

    size_t nbytes = bh_size + mh_size + message_length;
    ssize_t nread = 0;
    if (io_socket->read_nbytes_cycle(nbytes, &nread) == -1)
    {
        return -1;
    }
    const char *p = io_buffer->get_data() + bh_size + mh_size;

    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_STRING)
    {
        return -1;
    }

    int16_t len = 0;
    if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
    {
        return -1;
    }

    if (len <= 0)
    {
        return -1;
    }

    p += 2;
    std::string command_name;
    command_name.append(p, len);

    p += len;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_STRING)
    {
        return -1;
    }

    if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
    {
        return -1;
    }

    if (len <= 0)
    {
        return -1;
    }

    p += 2;
    std::string content;
    content.append(p, len);

    p += len;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != 0x08)
    {
        return -1;
    }

    int32_t count = 0;
    if (p + 4 > io_buffer->get_data() + io_buffer->size() || be2se_4bytes(p, (char *)&count) == -1)
    {
        return -1;
    }

    if (count <= 0)
    {
        return -1;
    }

    p += 4;
    
    for (int i = 0; i < count; i++)
    {
        if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
        {
            return -1;
        }
        if (len <= 0)
        {
            return -1;
        }
        p += 2;
        std::string key;
        key.append(p, len);
        p += len;

        if (p >= io_buffer->get_data() + io_buffer->size())
        {
            return -1;
        }

        if (*p == 0x00)
        {
            p++;
            double value = 0;
            if (p + 8 > io_buffer->get_data() + io_buffer->size() || be2se_8bytes(p, (char *)&value) == -1)
            {
                return -1;
            }
            p += 8;
            Value_Object *value_object = new Value_Num(value);
            set_data_frame.push_back(std::pair<std::string, Value_Object *>(key, value_object));
        }
        else if (*p == 0x01)
        {
            p++;
            if (p >= io_buffer->get_data() + io_buffer->size())
            {
                return -1;
            }
            Value_Object *value_object = new Value_0x01(*p++);
            set_data_frame.push_back(std::pair<std::string, Value_Object *>(key, value_object));
        }
        else if (*p == 0x02)
        {
            p++;
            if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
            {
                return -1;
            }
            if (len <= 0)
            {
                return -1;
            }
            p += 2;
            Value_Object *value_object = new Value_String(p, len);
            set_data_frame.push_back(std::pair<std::string, Value_Object *>(key, value_object));
            p += len;
        }
        else
        {
            LOG_ERROR("marker error");
            return -1;
        }
    }

    return 0;
}

IdentifyClient::IdentifyClient(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer) : stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

IdentifyClient::~IdentifyClient() {}

int IdentifyClient::read_header_get_payloadLength(std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    int fmt = 0, csid = 0;
    size_t bh_size = 0;
    if (read_basic_header(&fmt, &csid, &bh_size) == -1)
    {
        return -1;
    }

    size_t mh_size = 0;
    if (read_message_header(fmt, csid, bh_size, &mh_size, received_message_length_buffer) == -1)
    {
        return -1;
    }

    if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
    {
        return -1;
    }
    size_t v_size = received_message_length_buffer[csid].size();
    int message_length = received_message_length_buffer[csid][v_size - 1];

    return message_length;
}

GetStreamLength::GetStreamLength(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer) : stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

GetStreamLength::~GetStreamLength() {}

int GetStreamLength::read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    if (message_length <= 0)
    {
        if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
        {
            return -1;
        }
        size_t v_size = received_message_length_buffer[csid].size();
        message_length = received_message_length_buffer[csid][v_size - 1];
    }

    size_t nbytes = bh_size + mh_size + message_length;
    ssize_t nread = 0;
    if (io_socket->read_nbytes_cycle(nbytes, &nread) == -1)
    {
        return -1;
    }
    const char *p = io_buffer->get_data() + bh_size + mh_size;

    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_STRING)
    {
        return -1;
    }

    int16_t len = 0;
    if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
    {
        return -1;
    }

    if (len <= 0)
    {
        return -1;
    }

    p += 2;
    std::string command_name;
    command_name.append(p, len);

    p += len;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_NUM)
    {
        return -1;
    }

    int64_t transaction_id = 0;
    if (p + 8 > io_buffer->get_data() + io_buffer->size() || be2se_8bytes(p, (char *)&transaction_id) == -1)
    {
        return -1;
    }
    
    p += 8;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAFE_NULL_OBJECT)
    {
        return -1;
    }

    copy(command_name, transaction_id);

    return 0;
}

PlayCommandMessage::PlayCommandMessage(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer) : stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

PlayCommandMessage::~PlayCommandMessage() {}

int PlayCommandMessage::read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    if (message_length <= 0)
    {
        if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
        {
            return -1;
        }
        size_t v_size = received_message_length_buffer[csid].size();
        message_length = received_message_length_buffer[csid][v_size - 1];
    }

    size_t nbytes = bh_size + mh_size + message_length;
    ssize_t nread = 0;
    if (io_socket->read_nbytes_cycle(nbytes, &nread) == -1)
    {
        return -1;
    }
    const char *p = io_buffer->get_data() + bh_size + mh_size;

    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_STRING)
    {
        return -1;
    }

    int16_t len = 0;
    if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
    {
        return -1;
    }

    if (len <= 0)
    {
        return -1;
    }

    p += 2;
    std::string command_name;
    command_name.append(p, len);

    p += len;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAGE_NUM)
    {
        return -1;
    }

    int64_t transaction_id = 0;
    if (p + 8 > io_buffer->get_data() + io_buffer->size() || be2se_8bytes(p, (char *)&transaction_id) == -1)
    {
        return -1;
    }
    
    p += 8;
    if (p >= io_buffer->get_data() + io_buffer->size() || *p++ != MESSAFE_NULL_OBJECT)
    {
        return -1;
    }

    copy(command_name, transaction_id);

    return 0;
}

SrcPCUCStreamBegin::SrcPCUCStreamBegin(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer) : stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer),
IO_Message(client, stfd_client, io_socket, io_buffer) {}

SrcPCUCStreamBegin::~SrcPCUCStreamBegin() {}

int SrcPCUCStreamBegin::write_payload()
{
    int16_t type = 0;
    io_buffer->push_back((char *)&type, 2);
    int32_t data = 1;
    int32_t dst = 0;
    if (se2be_4bytes((char *)&data, (char *)&dst) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&dst, 4);

    return 0;   
}

PullStreamOnStatus::PullStreamOnStatus(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, std::string command, double transaction_id, MESSAGE_OBJECT_TYPE type) :
io_buffer(io_buffer), command(command), transaction_id(transaction_id), type(type), IO_Message(client, stfd_client, io_socket, io_buffer) {init();}

PullStreamOnStatus::~PullStreamOnStatus()
{
    for (int i = 0; i < vec.size(); i++)
    {
        for (const auto &it : vec[i])
        {
            if (it.second)
            {
                delete it.second;
            }
        }
    }
}

int PullStreamOnStatus::write_payload()
{
    char marker_str = 0x02;
    io_buffer->push_back(&marker_str, 1);
    int16_t len = command.size();
    char buffer_len[2] = {0};
    if (se2be_2bytes((char *)&len, buffer_len) == -1)
    {
        return -1;
    }
    io_buffer->push_back(buffer_len, 2);
    io_buffer->push_back(command.c_str(), len);

    char marker_num = 0x00;
    io_buffer->push_back(&marker_num, 1);
    int64_t buffer_id = 0;
    memcpy(&buffer_id, &transaction_id, sizeof(transaction_id));
    char dst[8] = {0};
    if (se2be_8bytes((char *)&buffer_id, dst) == -1)
    {
        return -1;
    }
    io_buffer->push_back(dst, 8);

    char marker_0x05 = 0x05;
    io_buffer->push_back(&marker_0x05, 1);

    for (int i = 0; i < vec.size(); i++)
    {
        char marker_object = 0x03;
        io_buffer->push_back(&marker_object, 1);
        for (int j = 0; j < vec[i].size(); j++)
        {
            int16_t len_key = vec[i][j].first.size();
            char buffer_key_len[2] = {0};
            if (se2be_2bytes((char *)&len_key, buffer_key_len) == -1)
            {
                return -1;
            }
            io_buffer->push_back(buffer_key_len, 2);
            io_buffer->push_back(vec[i][j].first.c_str(), (size_t)len_key);
            
            if (dynamic_cast<Value_String *>(vec[i][j].second))
            {
                io_buffer->push_back(&marker_str, 1);
                Value_String *value_string = dynamic_cast<Value_String *>(vec[i][j].second);
                int16_t len_value_str = (int16_t)value_string->value.size();
                char buffer_len_value[2] = {0};
                if (se2be_2bytes((char *)&len_value_str, buffer_len_value) == -1)
                {
                    return -1;
                }
                io_buffer->push_back(buffer_len_value, 2);
                io_buffer->push_back(value_string->value.c_str(), (size_t)len_value_str);
            }
            else if (dynamic_cast<Value_Num *>(vec[i][j].second))
            {
                io_buffer->push_back(&marker_num, 1);
                Value_Num *value_num = dynamic_cast<Value_Num *>(vec[i][j].second);
                char buffer_value_num[8] = {0};
                if (se2be_8bytes((char *)&value_num->value, buffer_value_num) == -1)
                {
                    return -1;
                }
                io_buffer->push_back(buffer_value_num, 8);
            }
            else if (dynamic_cast<Value_Len *>(vec[i][j].second))
            {
                char marker_len = 0x08;
                io_buffer->push_back(&marker_len, 1);
                char buffer_0x08_value[4] = {0};
                Value_Len *value_len = dynamic_cast<Value_Len *>(vec[i][j].second);
                if (se2be_4bytes((char *)&value_len->value, buffer_0x08_value) == -1)
                {
                    return -1;
                }
                io_buffer->push_back(buffer_0x08_value, 4);
            }
            else
            {
                LOG_ERROR("write_payload failed, the value type does not match");
                return -1;
            }
        }
        int16_t flag_0x00 = 0x00;
        char buffer_flag_0x00[2] = {0};    
        if (se2be_2bytes((char *)&flag_0x00, buffer_flag_0x00) == -1)
        {
            return -1;
        }
        io_buffer->push_back(buffer_flag_0x00, 2);
        char marker_0x09 = 0x09;
        io_buffer->push_back(&marker_0x09, 1);
    }

    if (vec.size() > 1)
    {
        int16_t flag_0x00 = 0x00;
        char buffer_flag_0x00[2] = {0};
        if (se2be_2bytes((char *)&flag_0x00, buffer_flag_0x00) == -1)
        {
            return -1;
        }
        io_buffer->push_back(buffer_flag_0x00, 2);
        char marker_0x09 = 0x09;
        io_buffer->push_back(&marker_0x09, 1);
    }

    return 0;
}

void PullStreamOnStatus::init()
{
    std::vector<std::pair<std::string, Value_Object *>> vec1;
    const char *value = "status";
    Value_Object *value_object = new Value_String(value, strlen(value));
    vec1.push_back(std::pair<std::string, Value_Object *>("level", value_object));

    if (type == PULL_ONSTATUS1)
    {
        value = "NetStream.Play.Reset";
    }
    else if (type == PULL_ONSTATUS2)
    {
        value = "NetStream.Play.Start";
    }
    value_object = new Value_String(value, strlen(value));
    vec1.push_back(std::pair<std::string, Value_Object *>("code", value_object));

    if (type == PULL_ONSTATUS1)
    {
        value = "Playing and resetting stream.";
    }
    else if (type == PULL_ONSTATUS2)
    {
        value = "Started playing stream.";
    }
    value_object = new Value_String(value, strlen(value));
    vec1.push_back(std::pair<std::string, Value_Object *>("description", value_object));

    value = "stream";
    value_object = new Value_String(value, strlen(value));
    vec1.push_back(std::pair<std::string, Value_Object *>("details", value_object));

    value = "ASAICiss";
    value_object = new Value_String(value, strlen(value));
    vec1.push_back(std::pair<std::string, Value_Object *>("clientid", value_object));

    vec.push_back(vec1);
}

RtmpSampleAccess::RtmpSampleAccess(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, std::string command) :
io_buffer(io_buffer), command(command), IO_Message(client, stfd_client, io_socket, io_buffer) {}

RtmpSampleAccess::~RtmpSampleAccess() {}

int RtmpSampleAccess::write_payload()
{
    char marker_0x02 = 0x02;
    io_buffer->push_back(&marker_0x02, 1);
    const char *c_name = "|RtmpSampleAccess";
    int16_t len = (int16_t)strlen(c_name);
    int16_t len_dst = 0;
    if (se2be_2bytes((char *)&len, (char *)&len_dst) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&len_dst, 2);
    io_buffer->push_back(c_name, len);
    char marker_0x01 = 0x01;
    io_buffer->push_back(&marker_0x01, 1);
    char value = 0;
    io_buffer->push_back(&value, 1);
    io_buffer->push_back(&marker_0x01, 1);
    io_buffer->push_back(&value, 1);

    return 0;
}

PullStreamOnStatus3::PullStreamOnStatus3(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, std::string command) :
io_buffer(io_buffer), command(command), IO_Message(client, stfd_client, io_socket, io_buffer) {}

PullStreamOnStatus3::~PullStreamOnStatus3() {}

int PullStreamOnStatus3::write_payload()
{
    char marker_0x02 = 0x02;
    io_buffer->push_back(&marker_0x02, 1);
    const char *c_name = "onStatus";
    int16_t len = (int16_t)strlen(c_name);
    int16_t len_dst = 0;
    if (se2be_2bytes((char *)&len, (char *)&len_dst) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&len_dst, 2);
    io_buffer->push_back(c_name, len);
    char marker_0x03 = 0x03;
    io_buffer->push_back(&marker_0x03, 1);
    c_name = "code";
    len  = strlen(c_name);
    if (se2be_2bytes((char *)&len, (char *)&len_dst) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&len_dst, 2);
    io_buffer->push_back(c_name, len);
    io_buffer->push_back(&marker_0x02, 1);
    c_name = "NetStream.Data.Start";
    len = strlen(c_name);
    if (se2be_2bytes((char *)&len, (char *)&len_dst) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&len_dst, 2);
    io_buffer->push_back(c_name, len);
    int16_t flag_0x00 = 0x00;
    io_buffer->push_back((char *)&flag_0x00, 2);
    char marker_0x09 = 0x09;
    io_buffer->push_back(&marker_0x09, 1);

    return 0;
}

OnMetaData::OnMetaData(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer, std::string command) :
io_buffer(io_buffer), command(command), IO_Message(client, stfd_client, io_socket, io_buffer) {}
    
OnMetaData::~OnMetaData() {}

int OnMetaData::write_payload()
{
    char marker_str = 0x02;
    io_buffer->push_back(&marker_str, 1);
    size_t command_len = command.size();
    int16_t command_len_dst = 0;
    if (se2be_2bytes((char *)&command_len, (char *)&command_len_dst) == -1)
    {
        return -1;
    }
    io_buffer->push_back((char *)&command_len_dst, 2);
    io_buffer->push_back(command.data(), command.size());

    char marker_num = 0x00;

    std::string server("srs(simple rtmp server)0.1");
    Value_Object *value_object = new Value_String(server.data(), server.size());
    set_data_frame.push_back(std::pair<std::string, Value_Object *>("server", value_object));
    std::vector<std::vector<std::pair<std::string, Value_Object *>>> vec;
    vec.push_back(set_data_frame);
    for (int i = 0; i < vec.size(); i++)
    {
        char marker_object = 0x03;
        io_buffer->push_back(&marker_object, 1);
        for (int j = 0; j < vec[i].size(); j++)
        {
            int16_t len_key = vec[i][j].first.size();
            char buffer_key_len[2] = {0};
            if (se2be_2bytes((char *)&len_key, buffer_key_len) == -1)
            {
                return -1;
            }
            io_buffer->push_back(buffer_key_len, 2);
            io_buffer->push_back(vec[i][j].first.c_str(), (size_t)len_key);
            
            if (dynamic_cast<Value_String *>(vec[i][j].second))
            {
                io_buffer->push_back(&marker_str, 1);
                Value_String *value_string = dynamic_cast<Value_String *>(vec[i][j].second);
                int16_t len_value_str = (int16_t)value_string->value.size();
                char buffer_len_value[2] = {0};
                if (se2be_2bytes((char *)&len_value_str, buffer_len_value) == -1)
                {
                    return -1;
                }
                io_buffer->push_back(buffer_len_value, 2);
                io_buffer->push_back(value_string->value.c_str(), (size_t)len_value_str);
            }
            else if (dynamic_cast<Value_Num *>(vec[i][j].second))
            {
                io_buffer->push_back(&marker_num, 1);
                Value_Num *value_num = dynamic_cast<Value_Num *>(vec[i][j].second);
                double buffer_value_num = 0;
                memcpy(&buffer_value_num, &value_num->value, 8);
                double dst_buffer = 0;
                if (se2be_8bytes((char *)&buffer_value_num, (char *)&dst_buffer) == -1)
                {
                    return -1;
                }
                io_buffer->push_back((char *)&dst_buffer, 8);
            }
            else if (dynamic_cast<Value_0x01 *>(vec[i][j].second))
            {
                char marker = 0x01;
                io_buffer->push_back(&marker, 1);
                Value_0x01 *value_0x01 = dynamic_cast<Value_0x01 *>(vec[i][j].second);
                io_buffer->push_back(&(value_0x01->value), 1);
            }
            else
            {
                LOG_ERROR("write_payload failed, the value type does not match");
                return -1;
            }
        }
        int16_t flag_0x00 = 0x00;
        char buffer_flag_0x00[2] = {0};    
        if (se2be_2bytes((char *)&flag_0x00, buffer_flag_0x00) == -1)
        {
            return -1;
        }
        io_buffer->push_back(buffer_flag_0x00, 2);
        char marker_0x09 = 0x09;
        io_buffer->push_back(&marker_0x09, 1);
    }

    if (vec.size() > 1)
    {
        int16_t flag_0x00 = 0x00;
        char buffer_flag_0x00[2] = {0};
        if (se2be_2bytes((char *)&flag_0x00, buffer_flag_0x00) == -1)
        {
            return -1;
        }
        io_buffer->push_back(buffer_flag_0x00, 2);
        char marker_0x09 = 0x09;
        io_buffer->push_back(&marker_0x09, 1);
    }


    return 0;
}