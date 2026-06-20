module;

#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

#include "engine_core_api.h"

module Logger;

import std;

namespace Engine
{
	FileLogSink::FileLogSink()
	{
		_logFile = std::ofstream("EngineLog.txt", std::ios::out);
	}

	void FileLogSink::Write(LogLevel /*level*/, std::string_view header, std::string_view msg)
	{
		_logFile << header << msg << "\n";
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

	LogManager::LogManager()
	{
		_sinks.reserve(2);
		_sinks.emplace_back(std::make_unique<ConsoleLogSink>());
		_sinks.emplace_back(std::make_unique<FileLogSink>());
	}

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

	void DoLog(LogLevel level, std::string_view file, unsigned int line, std::string_view formattedMsg)
	{
		auto header = std::format("{}:{} [{}] ", file, line, level);
		GlobalLoggerInstance.LogMethod(level, header, formattedMsg);
	}

	// Definition of the global logger instance. All engine modules and game DLLs
	// resolve to this single object in engine-core.dll via __declspec(dllimport).
	ENGINE_CORE_API LogManager GlobalLoggerInstance;

} // namespace Engine
