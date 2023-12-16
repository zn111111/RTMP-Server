#ifndef __IO_MESSAGE_H__
#define __IO_MESSAGE_H__

#include <string>
#include <unordered_map>
#include "st.h"
#include "io_socket.h"
#include "value_object.h"
#include <vector>

extern int curr_csid;
extern std::vector<std::pair<std::string, Value_Object *>> set_data_frame;

typedef struct Message_Header
{
    Message_Header();
    ~Message_Header();

    void clear();

    int header_length;
    int timestamp;
    int timestamp_delta;
    int message_length;
    char message_type;
    int stream_id;
}Message_Header;

extern std::unordered_map<int, Message_Header> csid_header;
extern std::string audio_video_buffer;

class Client;
class IO_Message
{
public:
    IO_Message(Client *client, st_netfd_t stfd_client, IO_Socket *io_socket, IO_Buffer *io_buffer);
    virtual ~IO_Message();

public:
    int recv_message();
    int write_message(int fmt, int csid, int timestamp, int message_length, char message_type, int stream_id);
    std::string get_command() const;
    void common_print();

    int recv_audio_video(size_t &bh_size, size_t &mh_size, int &csid);
    const char *get_payload() const;

private:
    virtual int read_payload(int csid, size_t bh_size, size_t mh_size, int message_length, int chunk_size);
    virtual int write_header(int fmt, int csid, int timestamp, int message_length, char message_type, int stream_id);
    virtual int write_payload();

protected:
    int read_object(size_t bh_size, size_t mh_size, const char *p);
    void copy(std::string command_name_, uint64_t transaction_id_);
    int write_object(const std::vector<std::vector<std::pair<std::string, Value_Object *>>> &vec);
    int get_payload_size(const std::vector<std::vector<std::pair<std::string, Value_Object *>>> &vec) const;
    int read_basic_header(int *fmt, int *csid, size_t *bh_size);
    int read_message_header(int fmt, int csid, size_t bh_size, size_t *mh_size);
    int recv_audio_video_payload(int csid);
    int read_audio_video_message_header(int fmt, int csid, size_t bh_size, size_t *mh_size);
    int read_audio_video_basic_header(int *fmt, int *csid, size_t *bh_size);

private:
    Client *client;
    st_netfd_t stfd_client;
    IO_Socket *io_socket;
    IO_Buffer *io_buffer;

    std::string command_name;
    uint64_t transaction_id;
    std::unordered_map<std::string, Value_Object *> object;
};

#endif