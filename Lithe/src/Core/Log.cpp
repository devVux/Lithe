#include "Log.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <filesystem>

namespace Lithe {

std::shared_ptr<spdlog::logger>
Logger::initLogger(const std::string& name, const std::string& logFile, size_t maxFileSize, size_t maxFiles) {
	try {
		// Create sinks (not loggers)
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_level(spdlog::level::trace);

		std::filesystem::path logPath = std::filesystem::path(LOG_DIR) / logFile;
		auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath.string(), maxFileSize, maxFiles);
		fileSink->set_level(spdlog::level::trace);

		// Create logger with both sinks
		spdlog::sinks_init_list sinks  = {consoleSink, fileSink};
		auto					logger = std::make_shared<spdlog::logger>(name, sinks);

		logger->set_level(spdlog::level::trace);
		logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v");
		logger->flush_on(spdlog::level::err);

		spdlog::register_logger(logger);
		spdlog::set_default_logger(logger);

		return logger;
	} catch (const spdlog::spdlog_ex& ex) {
		throw std::runtime_error("Logger initialization failed: " + std::string(ex.what()));
	}
}

} // namespace Lithe
