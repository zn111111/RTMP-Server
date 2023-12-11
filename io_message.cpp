#include <cstdio>
#include <cstring>
#include "io_message.h"
#include "util.h"
#include "socket_demo_define.h"
#include "client.h"

int curr_csid = 0;
std::vector<std::pair<std::string, Value_Object *>> set_data_frame;
std::unordered_map<int, Message_Header> csid_header;
std::string audio_video_buffer;

Message_Header::Message_Header() : header_length(-1), timestamp(-1), timestamp_delta(-1), message_length(-1), message_type(-1), stream_id(-1) {}
Message_Header::~Message_Header() {}

void Message_Header::clear()
{
    header_length = -1;
    timestamp = -1;
    timestamp_delta = -1;
    message_length = -1;
    message_type = -1;
    stream_id = -1;
}

IO_Message::IO_Message(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer)
: client(client), stfd_client(stfd_client), io_socket(io_socket), io_buffer(io_buffer)
{
    transaction_id = 0;
}

IO_Message::~IO_Message() {io_buffer->clear();}

int IO_Message::recv_message(std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    int fmt = 0, csid = 0;
    size_t bh_size = 0;
    if (read_basic_header(&fmt, &csid, &bh_size) == -1)
    {
        return -1;
    }
    printf("read basic header success, bh_size = %ld, fmt = %d, csid = %d\n", bh_size, fmt, csid);

    size_t mh_size = 0;
    if (read_message_header(fmt, csid, bh_size, &mh_size, received_message_length_buffer) == -1)
    {
        return -1;
    }

    if (read_payload(csid, bh_size, mh_size, 0, received_message_length_buffer) == -1)
    {
        return -1;
    }
    printf("recv message success\n");

    return 0;
}

int IO_Message::write_message(int fmt, int csid, int timestamp, int message_length, char message_type, int stream_id)
{
    if (write_header(fmt, csid, timestamp, message_length, message_type, stream_id) == -1)
    {
        return -1;
    }
    size_t header_size = io_buffer->size();
    if (write_payload() == -1)
    {
        return -1;
    }

    size_t message_size = io_buffer->size();

    // if (message_length != message_size - header_size)
    // {
    //     printf("message_length参数有误, 实际message_length = %ld\n", message_size - header_size);
    //     return -1;
    // }

    struct iovec iov[2];

    iov[1].iov_base = const_cast<char *>(io_buffer->get_data() + header_size);
    iov[1].iov_len = (message_size - header_size) < client->chunk_size ? (message_size - header_size) : client->chunk_size;

    char *pos = const_cast<char *>(io_buffer->get_data() + header_size);
    char header_fmt3 = 0;
    ssize_t nwrite = 0;
    ssize_t totol_payload_send = 0;
    while (pos < io_buffer->get_data() + message_size)
    {
        if (pos == io_buffer->get_data() + header_size)
        {
            iov[0].iov_base = const_cast<char *>(io_buffer->get_data());
            iov[0].iov_len = header_size;
        }
        else
        {
            header_fmt3 = ((0x03 << 6) | (int8_t)csid);
            iov[0].iov_base = &header_fmt3;
            iov[0].iov_len = 1;
        }

        if (io_socket->writev(iov, 2, &nwrite) == -1)
        {
            return -1;
        }
        if (pos == io_buffer->get_data() + header_size)
        {
            totol_payload_send += (nwrite - (ssize_t )header_size);
        }
        else
        {
            totol_payload_send += (nwrite - 1);
        }
        pos = const_cast<char *>(io_buffer->get_data() + header_size + totol_payload_send);

        iov[1].iov_base = pos;
        iov[1].iov_len = (message_size - header_size - totol_payload_send) < client->chunk_size
                            ? (message_size - header_size - totol_payload_send) : client->chunk_size;
    }

    return 0;
}

std::string IO_Message::get_command() const
{
    return command_name;
}

int IO_Message::read_basic_header(int *fmt, int *csid, size_t *bh_size)
{
    ssize_t nread = 0;
    *bh_size = 1;
    if (io_socket->read_nbytes_cycle(1, &nread) == -1)
    {
        return -1;
    }

    char bh = *(io_buffer->get_data());

    *fmt = ((bh >> 6) & 0x03);
    *csid = (bh & 0x3F);

    if (*csid <= 1)
    {
        if (*csid == 0)
        {
            if (io_socket->read_nbytes(2, &nread) == -1)
            {
                return -1;
            }

            *csid = 64 + *(io_buffer->get_data() + 1);
            *bh_size = 2;
            return 0;
        }
        else if (*csid == 1)
        {
            if (io_socket->read_nbytes(3, &nread) == -1)
            {
                return -1;
            }

            *csid = 64 + *(io_buffer->get_data() + 1) + *(io_buffer->get_data() + 2) * 256;
            *bh_size = 3;
            return 0;
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

int IO_Message::read_message_header(int fmt, int csid, size_t bh_size, size_t *mh_size, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    size_t mh_size_array[] = {11, 7, 3, 0};
    *mh_size = mh_size_array[fmt];

    size_t nbytes = bh_size + *mh_size;
    ssize_t nread = 0;
    if (io_socket->read_nbytes_cycle(nbytes, &nread) == -1)
    {
        return -1;
    }
    
    int timestamp = 0;
    int timestamp_delta = 0;
    int message_length;
    char message_type = 0;
    int stream_id = 0;
    const char *p = io_buffer->get_data() + bh_size;
    if (fmt <= 2)
    {
    
        char *pp = NULL;
        if (fmt == 0)
        {
            pp = (char *)(&timestamp);
        }
        else
        {
            pp = (char *)(&timestamp_delta);
        }
        pp[3] = 0;
        pp[2] = *p++;
        pp[1] = *p++;
        pp[0] = *p++;
        
        if (fmt > 0 && timestamp_delta >= 0xFFFFFF)
        {
            timestamp_delta = 0xFFFFFF;
        }

        if (fmt <= 1)
        {
            pp = (char *)(&message_length);
            pp[3] = 0;
            pp[2] = *p++;
            pp[1] = *p++;
            pp[0] = *p++;

            if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
            {
                std::vector<int> temp;
                temp.push_back(message_length);
                received_message_length_buffer.insert(std::pair<int, std::vector<int>>(csid, temp));
            }
            else
            {
                received_message_length_buffer[csid].push_back(message_length);
            }

            message_type = *p++;

            if (fmt < 1)
            {
                pp = (char *)(&stream_id);
                pp[0] = *p++;
                pp[1] = *p++;
                pp[2] = *p++;
                pp[3] = *p++;
            }
        }
    }

    if (timestamp_delta == 0xFFFFFF)
    {
        nbytes += 4;
        if (io_socket->read_nbytes_cycle(nbytes, &nread) == -1)
        {
            return -1;
        }
        p = io_buffer->get_data() + bh_size + *mh_size;
        char *pp = (char *)&timestamp_delta;
        pp[3] = *p++;
        pp[2] = *p++;
        pp[1] = *p++;
        pp[0] = *p++;
        *mh_size += 4;
    }

    timestamp += timestamp_delta;

    printf("read message header success, mh_size = %ld, timestamp = %d, message_length = %d, message_type = %d, stream_id = %d\n", *mh_size, timestamp, message_length, message_type, stream_id);

    return 0;
}

int IO_Message::read_audio_video_message_header(int fmt, int csid, size_t bh_size, size_t *mh_size, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    size_t mh_size_array[] = {11, 7, 3, 0};
    *mh_size = mh_size_array[fmt];

    size_t nbytes = bh_size + *mh_size;
    ssize_t nread = 0;
    if (io_socket->read_nbytes_cycle_av(nbytes, &nread) == -1)
    {
        return -1;
    }
    
    if (fmt == 3)
    {
        if (csid_header.find(csid) == csid_header.end())
        {
            return -1;
        }

        csid_header[csid].timestamp += csid_header[csid].timestamp_delta;

        return 0;
    }

    const char *p = audio_video_buffer.data() + bh_size;
    if (fmt <= 2)
    {
        if (csid_header.find(csid) == csid_header.end())
        {
            Message_Header header;
            csid_header.insert(std::pair<int, Message_Header>(csid, header));
        }
        char *pp = NULL;
        if (fmt == 0)
        {
            pp = (char *)&(csid_header[csid].timestamp);
        }
        else
        {
            pp = (char *)(&csid_header[csid].timestamp_delta);
        }
        pp[3] = 0;
        pp[2] = *p++;
        pp[1] = *p++;
        pp[0] = *p++;
        
        if (fmt == 0)
        {
            csid_header[csid].timestamp_delta = csid_header[csid].timestamp;
        }

        if (fmt > 0 && csid_header[csid].timestamp_delta >= 0xFFFFFF)
        {
            csid_header[csid].timestamp_delta = 0xFFFFFF;
        }

        if (fmt <= 1)
        {
            pp = (char *)(&csid_header[csid].message_length);
            pp[3] = 0;
            pp[2] = *p++;
            pp[1] = *p++;
            pp[0] = *p++;

            if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
            {
                std::vector<int> temp;
                temp.push_back(csid_header[csid].message_length);
                received_message_length_buffer.insert(std::pair<int, std::vector<int>>(csid, temp));
            }
            else
            {
                received_message_length_buffer[csid].push_back(csid_header[csid].message_length);
            }

            csid_header[csid].message_type = *p++;

            if (fmt < 1)
            {
                pp = (char *)(&csid_header[csid].stream_id);
                pp[0] = *p++;
                pp[1] = *p++;
                pp[2] = *p++;
                pp[3] = *p++;
            }
        }
    }

    if (csid_header[csid].timestamp_delta == 0xFFFFFF)
    {
        nbytes += 4;
        if (io_socket->read_nbytes_cycle_av(nbytes, &nread) == -1)
        {
            return -1;
        }
        p = audio_video_buffer.data() + bh_size + *mh_size;
        char *pp = (char *)&csid_header[csid].timestamp_delta;
        pp[3] = *p++;
        pp[2] = *p++;
        pp[1] = *p++;
        pp[0] = *p++;
        *mh_size += 4;
    }

    csid_header[csid].timestamp += csid_header[csid].timestamp_delta;

    return 0;
}

int IO_Message::read_audio_video_basic_header(int *fmt, int *csid, size_t *bh_size)
{
    ssize_t nread = 0;
    *bh_size = 1;
    if (io_socket->read_nbytes_cycle_av(1, &nread) == -1)
    {
        return -1;
    }

    char bh = *(audio_video_buffer.data());

    *fmt = ((bh >> 6) & 0x03);
    *csid = (bh & 0x3F);

    if (*csid <= 1)
    {
        if (*csid == 0)
        {
            if (io_socket->read_nbytes_av(2, &nread) == -1)
            {
                return -1;
            }

            *csid = 64 + *(audio_video_buffer.data() + 1);
            *bh_size = 2;
            return 0;
        }
        else if (*csid == 1)
        {
            if (io_socket->read_nbytes_av(3, &nread) == -1)
            {
                return -1;
            }

            *csid = 64 + *(audio_video_buffer.data() + 1) + *(audio_video_buffer.data() + 2) * 256;
            *bh_size = 3;
            return 0;
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

int IO_Message::read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
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
    if (csid_header[csid].message_type == 0x01)
    {
        return 0;
    }
    if (*p++ != MESSAGE_STRING)
    {
        return -1;
    }

    int16_t len = 0;
    if (be2se_2bytes(p, (char *)&len) == -1)
    {
        return -1;
    }

    if (len <= 0)
    {
        return -1;
    }

    p += 2;
    command_name.append(p, len);
    
    //不知道为什么，客户端发送的connect命令，object部分tcUrl对应的value里多了个字节，因此实际的payload比message header里的message length标识的大小大1个字节
    if (command_name == "connect")
    {
        nbytes += 1;
        if (io_socket->read_nbytes_cycle(nbytes, &nread) == -1)
        {
            return -1;
        }
    }

    p += len;
    if (*p++ != MESSAGE_NUM)
    {
        return -1;
    }

    if (be2se_8bytes(p, (char *)&transaction_id) == -1)
    {
        return -1;
    }
    
    p += 8;
    if (*p == MESSAGE_OBJECT)
    {
        p++;
        if (read_object(bh_size, mh_size, p) == -1)
        {
            return -1;
        }
    }
    else if (*p == 0x05)
    {
        p++;
        if (p < io_buffer->get_data() + io_buffer->size() && *p++ == 0x02)
        {
            int16_t len = 0;
            if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
            {
                return -1;
            }
            p += 2;
            if (len > 0)
            {
                std::string temp(p, len);
                printf("%s\n", temp.c_str());
            }
            p += len;
        }
        if (p < io_buffer->get_data() + io_buffer->size() && *p++ == 0x02)
        {
            int16_t len = 0;
            if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len) == -1)
            {
                return -1;
            }
            p += 2;
            if (len > 0)
            {
                std::string temp(p, len);
                printf("%s\n", temp.c_str());
            }
            p += len;
        }
        return 0;
    }
    //其他标记有待实现

    //打印测试
    if (true)
    {
        if (command_name == "_checkbw")
        {
            printf("drop _checkbw\n");
            return 0;
        }
        printf("command_name = %s, transaction_id = %ld, ", command_name.c_str(), transaction_id);
        for (const auto &val : object)
        {
            printf("%s = ", (val.first).c_str());
            if (dynamic_cast<Value_Num *>(val.second))
            {
                printf("%f ", ((Value_Num *)(val.second))->value);
            }
            else
            {
                printf("%s ", ((Value_String *)(val.second))->value.c_str());
            }
        }
        printf("\n");
    }

    return 0;
}

int IO_Message::read_object(size_t bh_size, size_t mh_size, const char *p)
{
    bool tcurl_flag = 0;
    size_t size = io_buffer->size();
    ssize_t nread = 0;
    while (p < (io_buffer->get_data() + io_buffer->size()))
    {
        int16_t len = 0;
        if ((p + 2 > io_buffer->get_data() + io_buffer->size()) || be2se_2bytes(p, (char *)&len) == -1)
        {
            return -1;
        }
        if (len == 0x00)
        {
            p += 2;
            if (p < io_buffer->get_data() + io_buffer->size() && *p++ == MESSAGE_END_OF_OBJECT)
            {
                if (p >= io_buffer->get_data() + io_buffer->size())
                {
                    return 0;
                }
                int16_t end_flag = 0;
                if ((p + 2 > io_buffer->get_data() + io_buffer->size()) || be2se_2bytes(p, (char *)&end_flag) == -1)
                {
                    return -1;
                }
                if (end_flag == 0x00)
                {
                    return 0;
                }
                p += 2;
                if (p < io_buffer->get_data() + io_buffer->size() && *p++ == MESSAGE_OBJECT)
                {
                    continue;
                }
            }
            return -1;
        }
        else
        {
            p += 2;
            std::string key;
            if (p + len <= io_buffer->get_data() + io_buffer->size())
            {
                key.append(p, len);
            }
            else
            {
                return -1;
            }

            p += len;
            if (p < io_buffer->get_data() + io_buffer->size() && *p == MESSAGE_NUM)
            {
                p++;
                double value = 0;
                if (p + 8 > io_buffer->get_data() + io_buffer->size() || be2se_8bytes(p, (char *)&value) == -1)
                {
                    return -1;
                }
                Value_Object *value_object = new Value_Num(value);
                object.insert(std::pair<std::string, Value_Object *>(key, value_object));
                p += 8;
            }
            else if (p < io_buffer->get_data() + io_buffer->size() && *p == MESSAGE_STRING)
            {
                p++;
                uint16_t len_str = 0;
                if (p + 2 > io_buffer->get_data() + io_buffer->size() || be2se_2bytes(p, (char *)&len_str) == -1)
                {
                    return -1;
                }
                if (len_str <= 0)
                {
                    return -1;
                }
                p += 2;
                Value_Object *value_object = NULL;

                if (p + len_str > io_buffer->get_data() + io_buffer->size())
                {
                    return -1;
                }
                // if (key == "tcUrl")
                // {
                //     char temp[len_str];
                //     int index = 0;
                //     for (int i = 0; i < len_str + 1; i++)
                //     {
                //         if (p[i] != -61)
                //         {
                //             temp[index++] = p[i];
                //         }
                //     }
                //     value_object = new Value_String(temp, (size_t)len_str);
                //     len_str++;
                // }
                // else
                // {
                //     value_object = new Value_String(p, (size_t)len_str);
                // }
                value_object = new Value_String(p, (size_t)len_str);
                object.insert(std::pair<std::string, Value_Object *>(key, value_object));
                p += len_str;
            }
            else if (p < io_buffer->get_data() + io_buffer->size() && *p == MESSAGE_BOOLEAN)
            {
                p++;
                Value_Object *value_object = new Value_0x01(*p++);
                object.insert(std::pair<std::string, Value_Object *>(key, value_object));
            }
            else
            {
                printf("marker is neither 0x00 nor 0x02\n");
                return -1;
            }
        }
    }
    return -1;
}

int IO_Message::write_header(int fmt, int csid, int timestamp, int message_length, char message_type, int stream_id)
{
    size_t bh_size = 1;
    char *bh_header = NULL;
    if (csid >=2 && csid <= 63)
    {
        bh_header = new char[bh_size];
        bh_header[0] = (fmt << 6) | csid;
    }
    else if (csid <= 319)
    {
        bh_size = 2;
        bh_header = new char[bh_size];
        bh_header[0] = (fmt << 6) & 0xc0;
        bh_header[1] = csid - 64;
    }
    else
    {
        bh_size = 3;
        bh_header = new char[bh_size];
        bh_header[0] = (fmt << 6) | 0x01;
        int16_t temp = csid - 64;
        if (memcpy(bh_header + 1, &temp, 2) == NULL)
        {
            return -1;
        }
    }
    io_buffer->push_back(bh_header, bh_size);
    delete bh_header;

    char mh_size_array[] = {11, 7, 3, 0};
    size_t mh_size = mh_size_array[fmt];
    bool extendedstamp = false;
    if (mh_size > 0)
    {
        if (timestamp > 0xffffff)
        {
            extendedstamp = true;
            int value = 0xffffff;
            char ts[3] = {0};
            if (se2be_3bytes((char *)&value, ts) == -1)
            {
                return -1;
            }
            io_buffer->push_back(ts, 3);
        }
        else
        {
            char ts[3] = {0};
            if (se2be_3bytes((char *)&timestamp, ts) == -1)
            {
                return -1;
            }
            io_buffer->push_back(ts, 3);
        }

        if (mh_size >= 7)
        {
            char ml[3] = {0};
            if (se2be_3bytes((char *)&message_length, ml) == -1)
            {
                return -1;
            }
            io_buffer->push_back(ml, 3);

            io_buffer->push_back(&message_type, 1);
        }

        if (mh_size > 7)
        {
            io_buffer->push_back((char *)&stream_id, 4);
        }
    }

    if (extendedstamp)
    {
        char e_ts[4] = {0};
        if (se2be_4bytes((char *)&timestamp, e_ts) == -1)
        {
            return -1;
        }
        io_buffer->push_back(e_ts, 4);
        mh_size += 4;
    }

    return 0;
}

int IO_Message::write_payload()
{
    return 0;
}

void IO_Message::copy(std::string command_name_, uint64_t transaction_id_)
{
    command_name = command_name_;
    transaction_id = transaction_id_;
}

int IO_Message::write_object(const std::vector<std::vector<std::pair<std::string, Value_Object *>>> &vec)
{
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
                char marker_str = 0x02;
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
                char marker_num = 0x00;
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
                printf("write_payload failed, the value type does not match\n");
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

void IO_Message::common_print()
{
    printf("command_name = %s, transaction_id = %ld", command_name.c_str(), transaction_id);
    if (!object.empty())
    {
        printf(", ");
        for (const auto &val : object)
        {
            printf("%s = ", (val.first).c_str());
            if (dynamic_cast<Value_Num *>(val.second))
            {
                printf("%f ", ((Value_Num *)(val.second))->value);
            }
            else
            {
                printf("%s ", ((Value_String *)(val.second))->value.c_str());
            }
        }
    }
    printf("\n");
}

int IO_Message::recv_audio_video(size_t &bh_size, size_t &mh_size, int &csid, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    int fmt = 0;
    if (read_audio_video_basic_header(&fmt, &csid, &bh_size) == -1)
    {
        return -1;
    }
    printf("read basic header success, bh_size = %ld, fmt = %d, csid = %d\n", bh_size, fmt, csid);

    if (read_audio_video_message_header(fmt, csid, bh_size, &mh_size, received_message_length_buffer) == -1)
    {
        return -1;
    }

    printf("read message header success, mh_size = %ld, ", mh_size);

    if (csid_header[csid].message_length <= 0)
    {
        if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
        {
            return -1;
        }
        size_t v_size = received_message_length_buffer[csid].size();
        csid_header[csid].message_length = received_message_length_buffer[csid][v_size - 1];
    }

    if (csid_header[csid].timestamp != -1)
    {
        printf("timestamp = %d, ", csid_header[csid].timestamp);
    }
    if (csid_header[csid].timestamp_delta != -1)
    {
        printf("timestamp_delta = %d, ", csid_header[csid].timestamp_delta);
    }
    if (csid_header[csid].message_length != -1)
    {
        printf("message_length = %d, ", csid_header[csid].message_length);
    }
    if (csid_header[csid].message_type != -1)
    {
        printf("message_type = %d, ", csid_header[csid].message_type);
    }
    if (csid_header[csid].stream_id != -1)
    {
        printf("stream_id = %d, ", csid_header[csid].stream_id);
    }
    printf("\n");

    if (recv_audio_video_payload(csid, received_message_length_buffer) == -1)
    {
        return -1;
    }

    printf("recv message success\n");

    return 0;
}

int IO_Message::recv_audio_video_payload(int csid, std::unordered_map<int, std::vector<int>> &received_message_length_buffer)
{
    if (csid_header[csid].message_length <= 0)
    {
        if (received_message_length_buffer.find(csid) == received_message_length_buffer.end())
        {
            return -1;
        }
        size_t v_size = received_message_length_buffer[csid].size();
        csid_header[csid].message_length = received_message_length_buffer[csid][v_size - 1];
    }

    ssize_t nread = 0;
    while (nread < csid_header[csid].message_length)
    {
        ssize_t temp = 0;
        size_t n_bytes = 0;
        //payload大于接收窗口大小，就需要分多次接收
        //第一次接收到的是纯payload，因为头部已经接收g过了
        //从第二次开始接收到的payload前面就会带上头部（fmt=3，也就是只有basic header，没有message header）
        if (nread > 0)
        {
            n_bytes = ((csid_header[csid].message_length - nread) <= client->chunk_size) ? (csid_header[csid].message_length - nread + 1) : client->chunk_size + 1;
        }
        else
        {
            n_bytes = ((csid_header[csid].message_length - nread) <= client->chunk_size) ? (csid_header[csid].message_length - nread) : client->chunk_size;
        }
        n_bytes += audio_video_buffer.size();
        if (io_socket->read_nbytes_cycle_av(n_bytes, &temp) == -1)
        {
            return -1;
        }

        //如果不是第一次接收，就要去掉头部,此时temp表示接收到的payload大小
        //删掉buffer里的头部，否则payload里会多出一个字节的头部
        if (nread > 0)
        {
            temp--;

            size_t index = audio_video_buffer.size() - 1;
            index -= temp;
            audio_video_buffer.erase(index, 1);
        }
        
        nread += temp;
    }

    return 0;
}

const char *IO_Message::get_payload() const
{
    return io_buffer->get_data();
}

int IO_Message::get_payload_size(const std::vector<std::vector<std::pair<std::string, Value_Object *>>> &vec) const
{
    int payload_size = 3 + command_name.size() + 9;

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
                printf("get_payload_size failed, the value type does not match\n");
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