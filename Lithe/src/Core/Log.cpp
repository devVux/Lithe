#include "Log.hpp"

#include <filesystem>

namespace Lithe {

std::shared_ptr<spdlog::logger>
Logger::initLogger(const std::string& name, const std::string& logFile, size_t maxFileSize, size_t maxFiles) {

	try {
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_level(spdlog::level::trace);

		std::filesystem::path logPath = std::filesystem::path(LOG_DIR) / logFile;
		auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath, maxFileSize, maxFiles);
		fileSink->set_level(spdlog::level::trace);

		std::vector<spdlog::sink_ptr> sinks {consoleSink, fileSink};
		auto						  logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());

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
