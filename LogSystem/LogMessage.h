#pragma once
#include "LogCommon.h"

// 仅管理日志字符串，不负责输出
class LogMessage
{
public:
	LogMessage(LogLevel level, std::string& text);
	~LogMessage();
	LogMessage(LogMessage const& /*other*/) = delete;
	LogMessage& operator=(LogMessage const& /*other*/) = delete;
	static std::string getTimeStr();
	inline const std::string getText() const { return _text; }
	inline const LogLevel getLevel() const { return _level; }
	inline std::string getPrefix() const { return _prefix; }
	const std::string getMessage() const {return getPrefix() + getText(); }
	inline void setPrefix(std::string const& prefix) { _prefix = prefix; }
	inline uint32_t Size() const
	{
		assert(!_prefix.empty() && !_text.empty());
		return static_cast<uint32_t>(_prefix.size() + _text.size());
	}
private:
	const LogLevel _level; // 日志等级
	const std::string _text; // 日志内容
	std::string _prefix; // 前缀
};