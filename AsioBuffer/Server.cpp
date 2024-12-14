#include "Server.h"

Server::Server(asio::io_context& io_context, const std::string& ip, unsigned short port)
    : _io_context(io_context)
    , _acceptor(io_context
        , tcp::endpoint(asio::ip::address::from_string(ip), port))
{
    start_accept(); // 启动异步接收客户端连接
}

Server::~Server()
{
}

void Server::start_accept()
{
    auto new_connection = std::make_shared<Session>(_io_context);
    // 异步等待客户端连接
    _acceptor.async_accept(new_connection->Socket(),
        [this, new_connection](boost_system::error_code ec) {
            if (!ec)
            {
                std::cout << "A connection is established!" << std::endl;
                // 连接成功后，开始处理客户端请求
                new_connection->start();
                // 继续接受下一个连接
                start_accept();
            }
            else std::cerr << "Accept failed: " << ec.message() << std::endl;
        });
}