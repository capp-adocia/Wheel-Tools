#pragma once
#include "common.h"
#include "Session.h"

class Server
{
public:
    // 构造函数，传入 io_context, IP 地址和端口号
    Server(asio::io_context& io_context, const std::string& ip, unsigned short port);
    ~Server();
    // 启动异步接收客户端连接
    void start_accept();

private:
    asio::io_context& _io_context; // io_context 用于异步操作
    asio::ip::tcp::acceptor _acceptor;
};