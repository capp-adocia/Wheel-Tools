#include "AppenderFile.h"
#include "LogMessage.h"
#include "Log.h"

AppenderFile::AppenderFile(uint8_t _id, std::string const& filename, LogLevel level)
	: Appender(_id, filename, level, APPENDER_FILE)
    , _logDir(LOGINST->GetLogsDir())
{
}

AppenderFile::~AppenderFile()
{
    CloseFile();
}

FILE* AppenderFile::OpenFile(std::string const& filename, std::string const& mode)
{
    std::string fullPath = _logDir + filename;

    // 提取文件夹路径（去掉文件名部分）
    std::string dirPath = fullPath.substr(0, fullPath.find_last_of("/\\"));

    // 检查目录是否存在
    struct stat info;
    if (stat(dirPath.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR))
    {
        // 目录不存在，创建目录
        if (mkdir(dirPath.c_str(), 0755) != 0)
        {
            std::cerr << "Failed to create directory: " << dirPath << std::endl;
            return nullptr;
        }
    }

    // 使用 fopen_s 来打开文件
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, fullPath.c_str(), mode.c_str());
    if (err != 0 || file == nullptr)
    {
        std::cerr << "Failed to open log file: " << fullPath << std::endl;
        return nullptr;
    }

    return file;
}


void AppenderFile::CloseFile()
{
    if (logfile)
    {
        fclose(logfile);
        logfile = nullptr;
    }
}

void AppenderFile::_write(const std::shared_ptr<LogMessage> message)
{
    logfile = OpenFile(Appender::getFileName(), "a");  // 使用追加模式
    if (logfile == nullptr)
    {
        std::cerr << "Log file is not open!" << std::endl;
        return;
    }

    // 写入日志消息
    if (message && !message->getMessage().empty())
    {
        // 使用 fprintf 写入日志消息到文件
        fprintf(logfile, "%s\n", message->getMessage().c_str());
        fflush(logfile); // 强制刷新缓冲区，确保消息立即写入文件
        return;
    }
    std::cerr << "Invalid log message!" << std::endl;
}
