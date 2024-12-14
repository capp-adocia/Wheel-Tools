#include "Log.h"

Log::Log()
	: lowestLogLevel(LogLevel::LOG_LEVEL_ERROR)
    , id(0)
{
}

Log::~Log()
{
}

void Log::Initialize()
{
    Clear();
    lowestLogLevel = LogLevel::LOG_LEVEL_ERROR;
    _logsDir = LOG_ROOT; // 根路径
    if ((_logsDir.at(_logsDir.length() - 1) != '/') && (_logsDir.at(_logsDir.length() - 1) != '\\'))
    {
        _logsDir.push_back('/');
    }
    struct stat info;
    if (stat(LOG_ROOT, &info) != 0 || !(info.st_mode & S_IFDIR))
    {
        // 目录不存在，创建目录
        if (mkdir(LOG_ROOT, 0755) != 0)
        {
            std::cerr << "Failed to create directory: " << LOG_ROOT << std::endl;
        }
    }
}

void Log::Clear()
{
    fileAppenders.clear();
    consoloAppenders.clear();
}

bool Log::ShouldLog(std::string const& type, LogLevel level) const
{
    if (level < lowestLogLevel)
        return false;
    return true;

    //LogLevel logLevel = logger->getLogLevel();
    //return logLevel != LOG_LEVEL_DISABLED && logLevel <= level;
}

void Log::HandleFilter(std::string const& filter)
{
    if (filter.empty()) assert(false && "filter is empty");
    // filter的格式为 目录/目录/文件名
}

void Log::write(AppenderType type, std::string const& filter, std::shared_ptr<LogMessage> msg)
{
    // 先对filter进行处理
    HandleFilter(filter);
    
    auto writeLog = [&](AppenderType type)
    {
        if (fileAppenders.find(filter) != fileAppenders.end() && type == APPENDER_FILE)
        {
            fileAppenders[filter]->write(std::move(msg));
        }
        else
        {
            std::unique_ptr<AppenderFile> fileAppender = std::make_unique<AppenderFile>(NextAppenderId(), filter);
            fileAppenders[filter] = std::move(fileAppender); // 存储新的
            fileAppenders[filter]->write(msg);
        }
        if (consoloAppenders.find(filter) != consoloAppenders.end() && type == APPENDER_CONSOLE)
        {
            consoloAppenders[filter]->write(std::move(msg));
        }
        else
        {
            std::unique_ptr<AppenderConsole> consoleAppender = std::make_unique<AppenderConsole>(NextAppenderId(), filter);
            consoloAppenders[filter] = std::move(consoleAppender);
            consoloAppenders[filter]->write(msg);
        }
    };
    if (type == APPENDER_NONE) return;
    if (type == APPENDER_INVALID) return;

    writeLog(type);
}
