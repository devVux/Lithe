#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
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
			const std::string& name = "Core",
			const std::string& logFile = "logs/core.log",
			size_t maxFileSize = 1048576 * 5,
			size_t maxFiles = 3
		);

};

}

#define LT_TRACE(...)    Lithe::Logger::coreLogger()->trace(__VA_ARGS__)
#define LT_DEBUG(...)    Lithe::Logger::coreLogger()->debug(__VA_ARGS__)
#define LT_INFO(...)     Lithe::Logger::coreLogger()->info(__VA_ARGS__)
#define LT_WARN(...)     Lithe::Logger::coreLogger()->warn(__VA_ARGS__)
#define LT_ERROR(...)    Lithe::Logger::coreLogger()->error(__VA_ARGS__)
#define LT_CRITICAL(...) Lithe::Logger::coreLogger()->critical(__VA_ARGS__)

