#pragma once

#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/pattern_formatter.h>

#include <iostream>

namespace Lithe {

	class Logger {

		public:

			Logger() = delete;
			static void init(const std::string& pattern = "[%T] [%^%l%$] %v") {
				if (!pCoreLogger) {
					try {
						pCoreLogger = spdlog::stdout_color_mt("Core");
						pCoreLogger->set_pattern("[%T] [%^%l%$] %v");
						pCoreLogger->set_level(spdlog::level::trace);
					} catch (const spdlog::spdlog_ex& ex) {
						std::cerr << "Logger initialization failed: " << ex.what() << '\n';
						throw;
					}
				}
			}

			static void shutdown() {
				pCoreLogger.reset();
			}

			static std::shared_ptr<spdlog::logger>& getCoreLogger() {
				if (!pCoreLogger) {
					init();
				}
				return pCoreLogger;
			}

		private:

			static inline std::shared_ptr<spdlog::logger> pCoreLogger = nullptr;
	
	};

	namespace Log {

		template <typename... Args>
		inline void FATAL(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			Logger::getCoreLogger()->critical(fmt, std::forward<Args>(args)...);
			std::terminate();
		}

		template <typename... Args>
		inline void ERR(fmt::format_string<Args...> fmt, Args&&... args) {
			Logger::getCoreLogger()->error(fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		inline void WARN(fmt::format_string<Args...> fmt, Args&&... args) {
			Logger::getCoreLogger()->warn(fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		inline void INFO(fmt::format_string<Args...> fmt, Args&&... args) {
			Logger::getCoreLogger()->info(fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		inline void DEBUG(fmt::format_string<Args...> fmt, Args&&... args) {
			Logger::getCoreLogger()->debug(fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		inline void TRACE(fmt::format_string<Args...> fmt, Args&&... args) {
			Logger::getCoreLogger()->trace(fmt, std::forward<Args>(args)...);
		}

	}

}
