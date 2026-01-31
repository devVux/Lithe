#pragma once

#include <expected>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>

namespace Lithe {

class Logger {

  public:

	static std::shared_ptr<spdlog::logger>& coreLogger() {
		static std::shared_ptr<spdlog::logger> logger = initLogger();
		return logger;
	}

  private:

	static std::shared_ptr<spdlog::logger> initLogger(
		const std::string& name		   = "Core",
		const std::string& logFile	   = "logs/core.log",
		size_t			   maxFileSize = 1'048'576 * 5,
		size_t			   maxFiles	   = 3
	);
};

#define LT_LOG_TRACE(...) Logger::coreLogger()->trace(__VA_ARGS__)
#define LT_LOG_DEBUG(...) Logger::coreLogger()->debug(__VA_ARGS__)
#define LT_LOG_INFO(...) Logger::coreLogger()->info(__VA_ARGS__)
#define LT_LOG_WARN(...) Logger::coreLogger()->warn(__VA_ARGS__)
#define LT_LOG_ERROR(...) Logger::coreLogger()->error(__VA_ARGS__)
#define LT_LOG_CRITICAL(...) Lithe::Logger::coreLogger()->critical(__VA_ARGS__)

} // namespace Lithe
