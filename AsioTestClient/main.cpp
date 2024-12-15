#include "common.h"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>  // 用于睡眠重连

enum CommandType : uint8_t
{
    Close = 0x00,
    Battle = 0x01
};

std::mutex socket_mutex;  // 保护socket的互斥锁

// 发送线程的工作函数
void send_message(tcp::socket& socket, const std::string& message) {
    try {
        std::lock_guard<std::mutex> lock(socket_mutex); // 确保每次只有一个线程能写
        char buffer[1024] = { 0 };
        short request_host_length = boost::asio::detail::socket_ops::host_to_network_short(message.size());
        memcpy(buffer, &request_host_length, 2);
        memcpy(buffer + 2, &message, message.size());

        asio::write(socket, asio::buffer(buffer, message.size() + 2));
        std::cout << "Message sent: " << message << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Send thread error: " << e.what() << std::endl;
    }
}

// 接收线程的工作函数
void receive_message(tcp::socket& socket) {
    try {
        while (true) {
            boost::system::error_code ec;
            char reply_head[2];
            boost::asio::read(socket, boost::asio::buffer(reply_head, 2));
            short msglen;
            memcpy(&msglen, reply_head, 2);
            //转为本地字节序
            msglen = boost::asio::detail::socket_ops::network_to_host_short(msglen);
            char msg[1024] = { 0 };
            boost::asio::read(socket, boost::asio::buffer(msg, msglen));
            std::cout << msg << std::endl;

            if (ec && ec != asio::error::eof) {
                throw boost::system::system_error(ec);
            }
            std::cout << "Received from server: ";
            std::cout.write(msg, msglen) << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Receive thread error: " << e.what() << std::endl;
    }
}

int main() {
    asio::io_context io_context;

    // 定义服务器 IP 和端口
    std::string server_ip = "127.0.0.1";  // 服务器 IP 地址
    unsigned short server_port = 10086;   // 服务器端口号

    tcp::socket socket(io_context);
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(server_ip, std::to_string(server_port));

    // 连接服务器，如果连接失败则重试
    while (true) {
        try {
            std::cout << "Trying to connect to server at " << server_ip << ":" << server_port << "..." << std::endl;
            asio::connect(socket, endpoints);
            std::cout << "Connected to server at " << server_ip << ":" << server_port << std::endl;
            break;  // 连接成功则跳出循环
        }
        catch (const boost::system::system_error& e) {
            std::cerr << "Connection failed: " << e.what() << std::endl;
            std::cout << "Retrying in 5 seconds..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));  // 等待 5 秒后重新尝试连接
        }
    }

    // 启动接收线程
    std::thread receive_thread(receive_message, std::ref(socket));

    // 启动多个发送线程
    std::vector<std::thread> send_threads;
    for (int i = 0; i < 4; ++i) {  // 假设我们启动4个线程来发送不同的消息
        send_threads.push_back(std::thread(send_message, std::ref(socket), "Hello " + std::to_string(i)));
    }

    // 等待所有发送线程结束
    for (auto& t : send_threads) {
        t.join();
    }

    // 等待接收线程结束（通常接收线程是永远在运行的，除非主动关闭连接）
    receive_thread.join();

    // 关闭连接
    socket.close();
    std::cout << "Connection closed." << std::endl;

    return 0;
}
