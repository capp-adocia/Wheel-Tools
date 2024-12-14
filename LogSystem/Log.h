#pragma once
#include "LogCommon.h"
#include "LogMessage.h"
#include <sstream>
#include "AppenderConsole.h"
#include "AppenderFile.h"

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#define LOGINST Log::Instance()

// 使用示例 意思是将server文件夹下的authserver
// LOG_INFO("server.authserver", "Using configuration file {}.", "filename");

class LogMessage;

class Log
{
    using AppenderFileMap = std::unordered_map<std::string, std::unique_ptr<AppenderFile>>;
    using AppenderConsoleMap = std::unordered_map<std::string, std::unique_ptr<AppenderConsole>>;
public:
    Log();
    ~Log();
    Log(Log const&) = delete;
    Log(Log&&) = delete;
    Log& operator=(Log const&) = delete;
    Log& operator=(Log&&) = delete;
    static Log* Instance()
    {
        static Log instance;
        return &instance;
    }
    void Initialize(); // 初始化日志系统
    void Clear(); // 清空日志map
    bool ShouldLog(std::string const& type, LogLevel level) const; // 根据级别判断是否需要记录日志
    void HandleFilter(std::string const& filter);
    template<typename... Args>
    void OutMessage(AppenderType type, std::string filter, const LogLevel level, const std::string& format, Args&&... args)
    {
        // 获取日志消息体
        std::string logText = this->OutMessageImpl(format, std::forward<Args>(args)...);
        // 记录日志文件或控制台输出
        write(type, filter, std::make_unique<LogMessage>(level, logText));
    }
    // 基础格式化函数，用于没有任何参数的情况
    std::string OutMessageImpl(const std::string& format) {
        return format;
    }
    // 辅助函数，处理不同类型的参数
    template <typename T, typename... Args>
    std::string OutMessageImpl(const std::string& format, T&& value, Args&&... args) {
        std::string result;
        size_t pos = format.find("{}"); // 查找占位符
        if (pos != std::string::npos) {
            result = format.substr(0, pos); // 获取占位符前的部分

            // 处理不同类型的值
            std::ostringstream oss;
            oss << value;  // 使用ostringstream来处理所有类型的参数
            result += oss.str(); // 将格式化后的字符串加入到结果中
            // 递归处理剩余的参数
            result += OutMessageImpl(format.substr(pos + 2), std::forward<Args>(args)...);
        }
        else {
            result = format;  // 如果没有占位符，直接返回格式化字符串
        }
        return result;
    }
    
    void write(AppenderType type, std::string const& filter, std::shared_ptr<LogMessage> msg); // 记录日志

    inline uint8_t NextAppenderId() { return id++; }
    inline std::string const& GetLogsDir() const { return _logsDir; }
    inline LogLevel getLowestLogLevel() const { return lowestLogLevel; }
    inline void setLowestLogLevel(LogLevel level) { lowestLogLevel = level; }
private:
    uint16_t id;
    LogLevel lowestLogLevel; // 默认设为最高级别
    std::string _logsDir; // 日志文件目录
    AppenderFileMap fileAppenders; // 文件日志输出器
    AppenderConsoleMap consoloAppenders; // 控制台日志输出器
};

#define LOG_MESSAGE_BODY(appendType__, filterType__, level__, ...)                        \
        do {                                                                \
            if (LOGINST->ShouldLog(filterType__, level__))                  \
                LOGINST->OutMessage(appendType__, filterType__, level__, __VA_ARGS__);    \
        } while (0)

#define LOG_DEBUG(appendType__, filterType__, ...) \
    LOG_MESSAGE_BODY(appendType__, filterType__, LOG_LEVEL_DEBUG, __VA_ARGS__)

#define LOG_INFO(appendType__, filterType__, ...)  \
    LOG_MESSAGE_BODY(appendType__, filterType__, LOG_LEVEL_INFO, __VA_ARGS__)

#define LOG_WARN(appendType__, filterType__, ...)  \
    LOG_MESSAGE_BODY(appendType__, filterType__, LOG_LEVEL_WARN, __VA_ARGS__)

#define LOG_ERROR(appendType__, filterType__, ...) \
    LOG_MESSAGE_BODY(appendType__, filterType__, LOG_LEVEL_ERROR, __VA_ARGS__)
