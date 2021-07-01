//
// Created by v4kst1z
//

#ifndef CPPNET_TCPCONNECTION_H
#define CPPNET_TCPCONNECTION_H


#include <memory>
#include <functional>
#include "Event.h"
#include "IOBuffer.h"

class Ipv4Addr;
class Looper;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using CallBack = std::function<void(const std::shared_ptr<TcpConnection>&)>;
    using MessageCallBack = std::function<void(const std::shared_ptr<TcpConnection>&, IOBuffer*)>;

    TcpConnection(int, Looper*, std::shared_ptr<Ipv4Addr>);

    void SetNewConnCallback(const CallBack &cb);
    void SetMessageCallBack(const MessageCallBack &cb);
    void SetCloseCallBack(const CallBack &cb);
    void SetSendDataCallBack(const CallBack &cb);
    void SetErrorCallBack(const CallBack &cb);

    void RunNewConnCallBack();
    void RunMessageCallBack();
    void RunCloseCallBack();
    void RunSendDataCallBack();
    void RunErrorCallBack();

    void OnRead();
    void OnWrite();
    void OnClose();

    void SendData(const void* data, size_t len);
    void SendData(const std::string& message);
    void SendData(IOBuffer& buffer);

    int GetConnFd();
    EventBase<Event>& GetEvent();
    const std::shared_ptr<Ipv4Addr> GetPeerAddr() const;

    ~TcpConnection();
private:
    int conn_fd_;
    bool half_close_;
    Looper *looper_;
    std::shared_ptr<Ipv4Addr> perr_addr_;
    EventBase<Event> conn_event_;

    CallBack new_conn_callback_;
    MessageCallBack message_callback_;
    CallBack send_data_callback_;
    CallBack close_callback_;
    CallBack error_callback_;

    IOBuffer input_buffer_;
    IOBuffer output_buffer;
};


#endif //CPPNET_TCPCONNECTION_H
