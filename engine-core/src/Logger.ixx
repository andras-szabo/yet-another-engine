module;	// This tells the compiler that what follows is the global fragment,
		// the only place in a module where it's allowrd to put #include
		// directives.

#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <source_location>
#include <string_view>
#include <vector>

#include "engine_core_api.h"

export module Logger;	// closes the global module fragment,
						// begins the module purview

namespace Logger
{
	export enum class LogLevel
	{
		Trace = 0,
		Debug = 1,
		Info = 2,
		Warning = 3,
		Error = 4
	};

	namespace ConsoleColor {
		constexpr const char* Reset = "\033[0m";
		constexpr const char* Red = "\033[91m";
		constexpr const char* Yellow = "\033[33m";
		constexpr const char* Green = "\033[32m";
		constexpr const char* Cyan = "\033[36m";
		constexpr const char* White = "\033[37m";
	}

	struct LogSink
	{
		virtual void Write(LogLevel level,
			std::string_view header,
			std::string_view formattedMessage) = 0;
		virtual ~LogSink() = default;
	};

	struct ConsoleLogSink : public LogSink
	{
		void Write(LogLevel, std::string_view, std::string_view) override;
	};

	struct FileLogSink : public LogSink
	{
		FileLogSink();
		void Write(LogLevel, std::string_view, std::string_view) override;

	private:
		std::ofstream _logFile;
	};

	FileLogSink::FileLogSink()
	{
		_logFile = std::ofstream("EngineLog.txt", std::ios::out);
	}

	void FileLogSink::Write(LogLevel /*level*/, std::string_view header, std::string_view msg)
	{
		_logFile << header << msg << "\n";
	}

	export class ENGINE_CORE_API LogManager
	{
	public:
		LogManager();
		LogManager(const LogManager&) = delete;
		LogManager& operator=(const LogManager&) = delete;
		LogManager(LogManager&&) = default;
		LogManager& operator=(LogManager&&) = default;

		void LogMethod(LogLevel, std::string_view header, std::string_view msg);

		LogLevel minLogLevel = LogLevel::Debug;

	private:
		// C4251: safe to suppress — _sinks is private and inaccessible to consumers.
		#pragma warning(suppress: 4251)
		std::vector<std::unique_ptr<LogSink>> _sinks;
	};
}

// Adding an std::formatter specialization for LogLevel, so that std::format
// can actually deal with this.
template<>
struct std::formatter<Logger::LogLevel> : std::formatter<std::string_view>
{
	auto format(Logger::LogLevel level, std::format_context& ctx) const
	{
		std::string_view name;
		switch (level)
		{
		case Logger::LogLevel::Trace:	name = "Trace"; break;
		case Logger::LogLevel::Debug:	name = "Debug"; break;
		case Logger::LogLevel::Info:	name = "Info"; break;
		case Logger::LogLevel::Warning:	name = "Warning"; break;
		case Logger::LogLevel::Error:	name = "Error"; break;
		default:
			name = "Unknown";
			break;
		}

		return std::formatter<std::string_view>::format(name, ctx);
	}
};

namespace Logger
{
	// Adding extern before the Log<> template so the compiler sees the
	// declaration before the template body references it.
	// Type is complete here (LogManager is fully defined above).
	extern ENGINE_CORE_API LogManager GlobalLoggerInstance;

	// Class template argument deduction (CTAD) guide; a hint to the compiler
	// that says: when someone constructs a Log with these arg types, deduce
	// the template parameters in the following way.
	//
	// Basically: the compiler cannot deduce template arguments for CLASSES
	// without help. (It can for function templates.)
	//
	// The deduction guide says: from these function parameters, deduce the 
	// following _type_ parameters:

	export
	template<typename... Args>
	struct Log
	{
		Log(LogLevel level,
			std::string_view msg,
			Args&&... args,
			std::source_location loc = std::source_location::current())
		{
			if (GlobalLoggerInstance.minLogLevel <= level)
			{
				auto header = std::format("{}:{} [{}] ",
					loc.file_name(),
					loc.line(),
					level);

				auto formattedMsg = std::vformat(msg, std::make_format_args(args...));
				GlobalLoggerInstance.LogMethod(level, header, formattedMsg);
			}
		}
	};

	template<typename... Args>
	Log(LogLevel, std::string_view, Args&&...) -> Log<Args...>;

	void LogManager::LogMethod(LogLevel logLevel, std::string_view header, std::string_view msg)
	{
		if (logLevel >= minLogLevel)
		{
			for (auto& sink : _sinks)
			{
				sink->Write(logLevel, header, msg);
			}
		}
	}

	LogManager::LogManager()
	{
		_sinks.reserve(2);
		_sinks.emplace_back(std::make_unique<ConsoleLogSink>());
		_sinks.emplace_back(std::make_unique<FileLogSink>());
	}

	void ConsoleLogSink::Write(LogLevel level,
		std::string_view header,
		std::string_view formattedMessage)
	{
		switch (level)
		{
			case LogLevel::Trace:	std::clog << ConsoleColor::Green; break;
			case LogLevel::Debug:	std::clog << ConsoleColor::Cyan; break;
			case LogLevel::Warning: std::clog << ConsoleColor::Yellow; break;
			case LogLevel::Error:	std::clog << ConsoleColor::Red; break;
		}

		std::clog << header << formattedMessage << ConsoleColor::Reset << "\n";
	}

	export ENGINE_CORE_API LogManager GlobalLoggerInstance;
}


