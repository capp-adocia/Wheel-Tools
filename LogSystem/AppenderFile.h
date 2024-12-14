#pragma once
#include "Appender.h"

class AppenderFile : public Appender
{
public:
    AppenderFile(uint8_t _id, std::string const& filename = LOG_ROOT, LogLevel level = LogLevel::LOG_LEVEL_INFO);
    ~AppenderFile();
    FILE* OpenFile(std::string const& name, std::string const& mode);
private:
    void CloseFile();
    void _write(const std::shared_ptr<LogMessage> message) override;
    FILE* logfile;
    std::string _fileName;
    std::string _logDir;
    //uint64_t _maxFileSize;
    //uint64_t _fileSize;
};