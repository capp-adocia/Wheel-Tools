#include "Log.h"

int main()
{
	LOGINST->Initialize();
	LOGINST->setLowestLogLevel(LogLevel::LOG_LEVEL_INFO);
	LOG_INFO(AppenderType::APPENDER_CONSOLE, "Hello/", "Hello {}!", "World");

	return 0;
}