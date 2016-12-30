#include "Logger.h"
#include <spdlog/spdlog.h>
#include <Windows.h>

namespace
{
	std::shared_ptr<spdlog::logger> Log;
	const char* LogExtension = ".log";

	std::string GetLogFilePath(const char* appName)
	{
		// %TEMP% folder
		char tempDirBuf[MAX_PATH];
		auto tempDirLength = GetTempPath(sizeof(tempDirBuf), tempDirBuf);
		std::string dir = (tempDirLength > 0 && tempDirLength <= sizeof(tempDirBuf)) ? tempDirBuf : "";
		return dir + appName + LogExtension;
	}
}

namespace Logger
{
	void Initialize(const char* appName, bool debugMode)
	{
		if (Log)
			return;

		// Always log to a file, but if debug mode is active then also display a console window
		std::vector<spdlog::sink_ptr> sinks;
		sinks.push_back(std::make_shared<spdlog::sinks::simple_file_sink_st>(GetLogFilePath(appName), /* truncate */ true));
		if (debugMode)
		{
			AllocConsole();
			sinks.push_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>());
		}

		Log = std::make_shared<spdlog::logger>(appName, sinks.begin(), sinks.end());
		spdlog::register_logger(Log);
		Log->set_level(debugMode ? spdlog::level::debug : spdlog::level::info);
		Log->flush_on(spdlog::level::debug); // needed or else the log will never flush
	}

	std::shared_ptr<spdlog::logger> Get()
	{
		return Log;
	}
}