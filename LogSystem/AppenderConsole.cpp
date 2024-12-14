#include "AppenderConsole.h"
#include "LogMessage.h"

AppenderConsole::AppenderConsole(uint8_t _id
	, std::string const& name
	, LogLevel level)
	: Appender(_id, name, level, APPENDER_CONSOLE)
{
}

void AppenderConsole::printLogMessage(LogLevel level, const std::string& prefix, const std::string& text)
{
    switch (level) {
    case LOG_LEVEL_DEBUG:
        std::cout << COLOR_DEBUG << prefix << COLOR_DEFAULT << text << std::endl;  // 青色
        break;
    case LOG_LEVEL_INFO:
        std::cout << COLOR_INFO << prefix << COLOR_DEFAULT << text << std::endl;  // 绿色
        break;
    case LOG_LEVEL_WARN:
        std::cout << COLOR_WARN << prefix << COLOR_DEFAULT << text << std::endl;  // 黄色
        break;
    case LOG_LEVEL_ERROR:
        std::cout << COLOR_ERROR << prefix << COLOR_DEFAULT << text << std::endl;  // 红色
        break;
    default:
        std::cout << prefix << text << std::endl;  // 默认没有颜色
        break;
    }
}

void AppenderConsole::_write(const std::shared_ptr<LogMessage> message)
{
	// 按照不同警告等级来输出不同颜色的日志信息
    printLogMessage(message->getLevel(), message->getPrefix(), message->getText());
}
