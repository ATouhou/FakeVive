#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace Logger
{
	void Initialize(const char* appName, bool debugMode);
	std::shared_ptr<spdlog::logger> Get();
}