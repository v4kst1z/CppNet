//
// Created by v4kst1z.
//

#include "UdpServer.h"

#include "Logger.h"
#include "Looper.h"

void UdpServer::SetMessageCallBack(const UdpServer::MessageCallBack &&cb) {
  message_callback_ = std::move(cb);
}

void UdpServer::SetSendDataCallBack(const UdpServer::CallBack &&cb) {
  send_data_callback_ = std::move(cb);
}