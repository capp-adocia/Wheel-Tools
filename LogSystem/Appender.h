#pragma once
#include "LogCommon.h"

class LogMessage;

// 日志输出器基类
class Appender
{
public:
    Appender(uint8_t id, std::string const& filename = "log", LogLevel level = LOG_LEVEL_DEBUG, AppenderType type = APPENDER_NONE);
    virtual ~Appender();
    inline uint8_t getId() const { return _id; }
    inline std::string const& getFileName() const { return _filename; }
    LogLevel getLogLevel() const { return _level; }
    virtual AppenderType getType() const { return _type; } // 获取日志输出类型
    char const* getLogLevelString(LogLevel level) const; // 获取日志等级字符串
    inline void setLogLevel(LogLevel level) { _level = level; }
    void write(const std::shared_ptr<LogMessage> message);
private:
    uint8_t _id;
    std::string _filename;
    LogLevel _level;
    AppenderType _type;
private:
    virtual void _write(const std::shared_ptr<LogMessage> /*message*/) = 0;
};