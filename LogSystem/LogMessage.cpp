#include "LogMessage.h"

LogMessage::LogMessage(LogLevel level, std::string& text)
    : _level(level)
    , _text(text)
{
}

LogMessage::~LogMessage()
{
}

std::string LogMessage::getTimeStr()
{
    // 获取当前日期时间
    time_t now = time(0);
    struct tm tstruct;

    if (localtime_s(&tstruct, &now) != 0) {
        std::cerr << "Error getting local time" << std::endl;
        return "";
    }

    char buf[80];
    // 修改格式字符串，包含时分秒
    strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", &tstruct);
    return std::string(buf);
}
