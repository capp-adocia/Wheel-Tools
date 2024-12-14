#pragma once
#include "Appender.h"
#define COLOR_DEFAULT "\033[0m"  // 默认颜色
#define COLOR_DEBUG "\033[36m"  // 调试日志 - 青色
#define COLOR_INFO "\033[32m"  // 调试日志 - 绿色
#define COLOR_WARN "\033[33m"  // 调试日志 - 黄色
#define COLOR_ERROR "\033[31m"  // 调试日志 - 红色

class AppenderConsole : public Appender
{
public:
    
    AppenderConsole(uint8_t _id, std::string const& name, LogLevel level = LogLevel::LOG_LEVEL_INFO);


private:
    void printLogMessage(LogLevel level, const std::string& prefix, const std::string& text);
    void _write(const std::shared_ptr<LogMessage> message) override;

};