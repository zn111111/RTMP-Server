#include <st.h>
#include "client.h"
#include "all_kinds_of_messages.h"
#include <unordered_map>
#include "log.h"

std::unordered_map<st_netfd_t, const char *> consumers;

Client::Client(st_netfd_t stfd_client)
: stfd_client(stfd_client)
{
    chunk_size = 128;
    io_buffer = new IO_Buffer;
    io_socket = new IO_Socket(stfd_client, io_buffer);
    rtmp = new RTMP_Protocol(stfd_client, io_socket, io_buffer);
}

Client::~Client()
{
    if (io_socket)
    {
        delete io_socket;
    }

    if (rtmp)
    {
        delete rtmp;
    }

    if (io_buffer)
    {
        delete io_buffer;
    }
}

void Client::cycle()
{
    if (st_thread_create(process_thread, this, 0, 0) == NULL)
    {
        LOG_ERROR("st thread create error");
        return;
    }
    LOG_INFO("st thread create success");
}

void *Client::process_thread(void *arg)
{
    if (arg)
    {
        Client *client = (Client *)(arg);
        if (client->rtmp->handshake() == -1)
        {
            return NULL;
        }
        LOG_INFO("handshake success");

        IO_Message *message_ptr = new ConnectMessage(client, client->stfd_client, client->io_socket, client->io_buffer);
        if (message_ptr->recv_message() == -1)
        {
            return NULL;
        }
        message_ptr->common_print();
        delete message_ptr;
        message_ptr = NULL;

        LOG_INFO("recv connect messsage success");

        message_ptr = new Set_Window_Size(client, client->stfd_client, client->io_socket, client->io_buffer, 2.5 * 1000 * 1000);
        if (message_ptr->write_message(0, 2, 0, 4, 5, 0) == -1)
        {
            return NULL;
        }
        delete message_ptr;
        message_ptr = NULL;
        LOG_INFO("Set window size success");

        message_ptr = new Set_Peer_Bandwidth(client, client->stfd_client, client->io_socket, client->io_buffer, 2.5 * 1000 * 1000, 2);
        if (message_ptr->write_message(0, 2, 0, 5, 6, 0) == -1)
        {
            return NULL;
        }
        delete message_ptr;
        message_ptr = NULL;

        LOG_INFO("Set peer bandwidth success");

        message_ptr = new Send_Object_Message(client, client->stfd_client, client->io_socket, client->io_buffer, "_result", 1, CONNECT_APP_RESPONSE);
        int message_length = dynamic_cast<Send_Object_Message *>(message_ptr)->get_payload_size();
        if (message_ptr->write_message(0, 3, 0, message_length, 20, 0) == -1)
        {
            return NULL;
        }
        delete message_ptr;
        message_ptr = NULL;

        LOG_INFO("send connect app response success");

        message_ptr = new Send_Object_Message(client, client->stfd_client, client->io_socket, client->io_buffer, "onBWDone", 0, ON_BW_DONE);
        message_length = dynamic_cast<Send_Object_Message *>(message_ptr)->get_payload_size();
        if (message_ptr->write_message(0, 3, 0, message_length, 20, 0) == -1)
        {
            return NULL;
        }
        delete message_ptr;
        message_ptr = NULL;

        LOG_INFO("send on bw done success");

        //push flow or pull flow ?
        IdentifyClient *identify_client= new IdentifyClient(client, client->stfd_client, client->io_socket, client->io_buffer);
        int ret = identify_client->read_header_get_payloadLength();
        std::string temp(client->io_buffer->get_data(), client->io_buffer->size());
        if (ret == -1)
        {
            delete identify_client;
            identify_client = NULL;
            return NULL;
        }
        //pull flow
        else if (ret == 4)
        {
            consumers.insert(std::pair<st_netfd_t, const char *>(client->stfd_client, NULL));

            delete identify_client;
            identify_client = NULL;

            ssize_t nread = 0;
            if (client->io_socket->read_nbytes_cycle(4, &nread) == -1)
            {
                return NULL;
            }
            client->io_buffer->clear();

            //receive Create Stream command message
            message_ptr = new CreateStream(client, client->stfd_client, client->io_socket, client->io_buffer);
            message_ptr->recv_message();
            message_ptr->common_print();
            delete message_ptr;
            message_ptr = NULL;

            //send Create Stream command message response
            message_ptr = new CreateStreamResponse(client, client->stfd_client, client->io_socket, client->io_buffer, 2, 1);
            message_ptr->write_message(0, 3, 0, 29, 20, 0);
            delete message_ptr;
            message_ptr = NULL;

            //receive _checkbw command message
            message_ptr = new CheckBW(client, client->stfd_client, client->io_socket, client->io_buffer);
            message_ptr->recv_message();
            delete message_ptr;
            message_ptr = NULL;

            //receive Get Stream Length command message
            message_ptr = new GetStreamLength(client, client->stfd_client, client->io_socket, client->io_buffer);
            message_ptr->recv_message();
            message_ptr->common_print();
            delete message_ptr;
            message_ptr = NULL;

            //receive play command message
            message_ptr = new PlayCommandMessage(client, client->stfd_client, client->io_socket, client->io_buffer);
            message_ptr->recv_message();
            message_ptr->common_print();
            delete message_ptr;
            message_ptr = NULL;

            message_ptr = new UnknownMessage(client, client->stfd_client, client->io_socket, client->io_buffer);
            message_ptr->recv_message();
            delete message_ptr;
            message_ptr = NULL;

            message_ptr = new SetChunkSize(client, client->stfd_client, client->io_socket, client->io_buffer, 4096);
            message_ptr->write_message(0, 2, 0, 4, 1, 0);
            client->chunk_size = 4096;
            delete message_ptr;
            message_ptr = NULL;

            LOG_INFO("set chunk size success");

            message_ptr = new SrcPCUCStreamBegin(client, client->stfd_client, client->io_socket, client->io_buffer);
            message_ptr->write_message(0, 2, 0, 6, 4, 0);
            delete message_ptr;
            message_ptr = NULL;

            LOG_INFO("send SrcPCUCStreamBegin success");

            message_ptr = new PullStreamOnStatus(client, client->stfd_client, client->io_socket, client->io_buffer, "onStatus", 0, PULL_ONSTATUS1);
            message_ptr->write_message(0, 5, 0, 154, 20, 1);
            delete message_ptr;
            message_ptr = NULL;

            LOG_INFO("send onstatus1 success");

            message_ptr = new PullStreamOnStatus(client, client->stfd_client, client->io_socket, client->io_buffer, "onStatus", 0, PULL_ONSTATUS2);
            message_ptr->write_message(0, 5, 0, 148, 20, 1);
            delete message_ptr;
            message_ptr = NULL;

            LOG_INFO("send onstatus2 success");

            message_ptr = new RtmpSampleAccess(client, client->stfd_client, client->io_socket, client->io_buffer, "|RtmpSampleAccess");
            message_ptr->write_message(0, 5, 0, 24, 18, 1);
            delete message_ptr;
            message_ptr = NULL;

            LOG_INFO("send |RtmpSampleAccess success");

            message_ptr = new PullStreamOnStatus3(client, client->stfd_client, client->io_socket, client->io_buffer, "onStatus");
            message_ptr->write_message(0, 5, 0, 44, 18, 1);
            delete message_ptr;
            message_ptr = NULL;

            LOG_INFO("send onstatus3 success");

            while (set_data_frame.empty())
            {
                st_usleep(0);
            }

            message_ptr = new OnMetaData(client, client->stfd_client, client->io_socket, client->io_buffer, "onMetaData");
            message_ptr->write_message(0, 4, 0, 395, 18, 1);
            delete message_ptr;
            message_ptr = NULL;

            LOG_INFO("send OnMetaData success");

            consumers.insert(std::pair<st_netfd_t, const char *>(client->stfd_client, NULL));

            while (true)
            {
                if (!consumers.begin()->second)
                {
                    st_usleep(0);
                    continue;                    
                }

                //调试使用
                // printf("buffer size: %ld\n", audio_video_buffer.size());
                // for (char c : audio_video_buffer)
                // {
                //     printf("%x ", (unsigned char)c);
                // }
                // printf("\n");


                //测试
                // printf("buffer size: %ld\n", audio_video_buffer.size());

                struct iovec iov[2];
                char header[16] = {0};

                iov[1].iov_base = const_cast<char *>(consumers.begin()->second + csid_header[curr_csid].header_length);
                iov[1].iov_len = csid_header[curr_csid].message_length < client->chunk_size ? (csid_header[curr_csid].message_length) : client->chunk_size;

                char *pos = const_cast<char *>(consumers.begin()->second) + csid_header[curr_csid].header_length;
                char header_fmt3 = 0;
                ssize_t nwrite = 0;
                ssize_t totol_payload_send = 0;
                while (pos < const_cast<char *>(consumers.begin()->second) + audio_video_buffer.size())
                {
                    if (pos == const_cast<char *>(consumers.begin()->second + csid_header[curr_csid].header_length))
                    {
                        int send_csid = 0;
                        if (csid_header[curr_csid].message_type == 0x08)
                        {
                            send_csid = 7;
                        }
                        else if (csid_header[curr_csid].message_type == 0x09)
                        {
                            send_csid = 6;
                        }
                        header[0] = ((0x00 << 6) | (int8_t)send_csid);
                        int timestamp = csid_header[curr_csid].timestamp;
                        if (timestamp >= 0xFFFFFF)
                        {
                            header[1] = 0xff;
                            header[2] = 0xff;
                            header[3] = 0xff;
                            iov[0].iov_len = 16;                         
                        }
                        else
                        {
                            header[1] = ((char *)&timestamp)[2];
                            header[2] = ((char *)&timestamp)[1];
                            header[3] = ((char *)&timestamp)[0];
                            iov[0].iov_len = 12;
                        }
                        int32_t payload_len = csid_header[curr_csid].message_length;
                        header[4] = ((char *)&payload_len)[2];
                        header[5] = ((char *)&payload_len)[1];
                        header[6] = ((char *)&payload_len)[0];
                        header[7] = csid_header[curr_csid].message_type;
                        int32_t streamid = csid_header[curr_csid].stream_id;
                        header[8] = ((char *)&streamid)[0];
                        header[9] = ((char *)&streamid)[1];
                        header[10] = ((char *)&streamid)[2];
                        header[11] = ((char *)&streamid)[3];
                        if (timestamp >= 0xFFFFFF)
                        {
                            header[12] = ((char *)&timestamp)[3];
                            header[13] = ((char *)&timestamp)[2];
                            header[14] = ((char *)&timestamp)[1];
                            header[15] = ((char *)&timestamp)[0];
                        }
                        iov[0].iov_base = header;
                    }
                    else
                    {
                        int send_csid = 0;
                        if (csid_header[curr_csid].message_type == 0x08)
                        {
                            send_csid = 7;
                        }
                        else if (csid_header[curr_csid].message_type == 0x09)
                        {
                            send_csid = 6;
                        }

                        header_fmt3 = ((0x03 << 6) | (int8_t)send_csid);
                        iov[0].iov_base = &header_fmt3;
                        iov[0].iov_len = 1;
                    }
                    if (client->io_socket->writev(iov, 2, &nwrite) == -1)
                    {
                        return NULL;
                    }
                    LOG_INFO("nwrite: %ld", nwrite);
                    if (pos == const_cast<char *>(consumers.begin()->second + csid_header[curr_csid].header_length))
                    {
                        totol_payload_send += (nwrite - 12);
                    }
                    else
                    {
                        totol_payload_send += (nwrite - 1);
                    }
                    pos = const_cast<char *>(consumers.begin()->second) + csid_header[curr_csid].header_length + totol_payload_send;

                    iov[1].iov_base = pos;
                    iov[1].iov_len = (csid_header[curr_csid].message_length - totol_payload_send) < client->chunk_size
                                        ? (csid_header[curr_csid].message_length - totol_payload_send) : client->chunk_size;
                }

                consumers.begin()->second = NULL;
                st_usleep(0);
            }
            
        }
        //push flow
        else
        {
            delete identify_client;
            identify_client = NULL;

            client->io_buffer->push_back(temp.c_str(), temp.size());

            //receive releaseStream command message
            message_ptr = new ReleaseStream(client, client->stfd_client, client->io_socket, client->io_buffer);
            message_ptr->recv_message();
            message_ptr->common_print();
            delete message_ptr;
            message_ptr = NULL;

            //send releaseStream command message response 
            message_ptr = new ReleaseStreamResponse(client, client->stfd_client, client->io_socket, client->io_buffer, 2);
            message_ptr->write_message(0, 3, 0, 21, 20, 0);
            delete message_ptr;
            message_ptr = NULL;

            LOG_INFO("send release stream response success");

            message_ptr = new SetChunkSize(client, client->stfd_client, client->io_socket, client->io_buffer, 4096);
            message_ptr->write_message(0, 2, 0, 4, 1, 0);
            client->chunk_size = 4096;
            delete message_ptr;
            message_ptr = NULL;

            LOG_INFO("set chunk size success");

            //receive FC Publish command message
            message_ptr = new FCPublish(client, client->stfd_client, client->io_socket, client->io_buffer);
            message_ptr->recv_message();
            message_ptr->common_print();
            delete message_ptr;
            message_ptr = NULL;

            //send FC Publish command message response
            message_ptr = new FCPublishResponse(client, client->stfd_client, client->io_socket, client->io_buffer, 3);
            message_ptr->write_message(0, 3, 0, 21, 20, 0);
            delete message_ptr;
            message_ptr = NULL;

            LOG_INFO("send fcpublish response success");

            //receive Create Stream command message
            message_ptr = new CreateStream(client, client->stfd_client, client->io_socket, client->io_buffer);
            message_ptr->recv_message();
            message_ptr->common_print();
            delete message_ptr;
            message_ptr = NULL;

            //send Create Stream command message response
            message_ptr = new CreateStreamResponse(client, client->stfd_client, client->io_socket, client->io_buffer, 4, 1);
            message_ptr->write_message(0, 3, 0, 29, 20, 0);
            delete message_ptr;
            message_ptr = NULL;

            LOG_INFO("send create stream response success");

            //receive _checkbw command message
            message_ptr = new CheckBW(client, client->stfd_client, client->io_socket, client->io_buffer);
            message_ptr->recv_message();
            delete message_ptr;
            message_ptr = NULL;

            message_ptr = new UnknownMessage(client, client->stfd_client, client->io_socket, client->io_buffer);
            message_ptr->recv_message();
            delete message_ptr;
            message_ptr = NULL;

            //receive publish command message
            message_ptr = new Publish(client, client->stfd_client, client->io_socket, client->io_buffer);
            message_ptr->recv_message();
            delete message_ptr;
            message_ptr = NULL;

            message_ptr = new OnFCPublish(client, client->stfd_client, client->io_socket, client->io_buffer, 0);
            message_length = 102;
            message_ptr->write_message(0, 3, 0, message_length, 20, 0);
            delete message_ptr;
            message_ptr = NULL;

            LOG_INFO("send onfcpublish response success");

            message_ptr = new OnStatus(client, client->stfd_client, client->io_socket, client->io_buffer, 0);
            message_length = 136;
            message_ptr->write_message(0, 3, 0, message_length, 20, 1);
            delete message_ptr;
            message_ptr = NULL;

            LOG_INFO("send onstatus success");

            //receive @setDataFrame data messsage
            message_ptr = new SetDataFrame(client, client->stfd_client, client->io_socket, client->io_buffer);
            message_ptr->recv_message();
            delete message_ptr;
            message_ptr = NULL;

            message_ptr = new IO_Message(client, client->stfd_client, client->io_socket, client->io_buffer);
            while (true)
            {
                size_t bh_size = 0, mh_size = 0;
                message_ptr->recv_audio_video(bh_size, mh_size, curr_csid);
                csid_header[curr_csid].header_length = bh_size + mh_size;

                //调试使用
                // printf("buffer size: %ld\n", audio_video_buffer.size());
                // for (char c : audio_video_buffer)
                // {
                //     printf("%x ", (unsigned char)c);
                // }
                // printf("\n");

                if (consumers.size() > 0)
                {
                    consumers.begin()->second = audio_video_buffer.data();
                }

                st_usleep(0);
                audio_video_buffer.clear();
            }
        }

    }
    return NULL;
}