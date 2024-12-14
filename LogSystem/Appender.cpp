#include "Appender.h"
#include "LogCommon.h"
#include "LogMessage.h"
#include <sstream>

Appender::Appender(uint8_t id, std::string const& filename, LogLevel level, AppenderType type)
	: _id(id)
	, _filename(filename)
	, _level(level)
	, _type(type)
{
}

Appender::~Appender()
{
}

char const* Appender::getLogLevelString(LogLevel level) const
{
    switch (level)
    {
    case LOG_LEVEL_ERROR:
        return "ERROR";
    case LOG_LEVEL_WARN:
        return "WARNNING";
    case LOG_LEVEL_INFO:
        return "INFOMATION";
    case LOG_LEVEL_DEBUG:
        return "DEBUG";
    default:
        return "DISABLED";
    }
}

void Appender::write(const std::shared_ptr<LogMessage> message)
{
    if (!_level) return;

    std::ostringstream ss;
    ss << message->getTimeStr() << ' ';
    ss << getLogLevelString(message->getLevel()) << " ";
    message->setPrefix(ss.str());
    _write(message);
}