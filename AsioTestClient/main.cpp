#include "common.h"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

enum CommandType : uint8_t
{
    Close = 0x00,
    Battle = 0x01
};

std::mutex socket_mutex;  // 保护socket的互斥锁

// 发送线程的工作函数
void send_message(CommandType type, tcp::socket& socket, const std::string& message) {
    try {
        std::lock_guard<std::mutex> lock(socket_mutex); // 确保每次只有一个线程能写
        const char* buffer;
        memcpy(buffer, &tlv.type, sizeof(tlv.type));  // 填充类型
        memcpy(buffer + sizeof(tlv.type), &tlv.length, sizeof(tlv.length));  // 填充长度
        memcpy(buffer + sizeof(tlv.type) + sizeof(tlv.length), tlv.value, tlv.length);  // 填充数据

        asio::write(socket, asio::buffer(message));
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
            std::vector<char> buffer(128);
            boost::system::error_code ec;
            size_t length = socket.read_some(asio::buffer(buffer), ec);

            if (ec && ec != asio::error::eof) {
                throw boost::system::system_error(ec);
            }

            std::cout << "Received from server: " << std::string(buffer.data(), length) << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Receive thread error: " << e.what() << std::endl;
    }
}

int main() {
    try {
        asio::io_context io_context;

        // 定义服务器 IP 和端口
        std::string server_ip = "127.0.0.1";  // 服务器 IP 地址
        unsigned short server_port = 10086;   // 服务器端口号

        // 创建 TCP 解析器
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(server_ip, std::to_string(server_port));

        // 创建 TCP socket
        tcp::socket socket(io_context);

        // 连接到服务器
        asio::connect(socket, endpoints);

        std::cout << "Connected to server at " << server_ip << ":" << server_port << std::endl;

        // 启动接收线程
        std::thread receive_thread(receive_message, std::ref(socket));

        // 启动多个发送线程
        std::vector<std::thread> send_threads;
        for (int i = 0; i < 8; ++i) {  // 假设我们启动8个线程来发送不同的消息
            send_threads.push_back(std::thread(send_message, std::ref(socket), 0x01, "Hello " + std::to_string(i)));
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
    }
    catch (const boost::system::system_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
