#include <vld.h>
#include <csignal>
#include "Server.h"

asio::io_context ioc;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received. Stopping server..." << std::endl;
    ioc.stop();
}

int main() {
    try {
        // 注册信号处理函数
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        Server server(ioc, "127.0.0.1", 10086);
        std::cout << "Server is running. Press Ctrl+C to stop." << std::endl;

        ioc.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    std::cout << "Server stopped gracefully." << std::endl;
    return 0;
}
