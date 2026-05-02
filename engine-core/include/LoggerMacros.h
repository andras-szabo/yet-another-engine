#pragma once

// Convenience macros for the Logger module.
//
// C++ modules cannot export macros, so these live in a companion header.
// Usage: alongside `import Logger;`, add `#include "LoggerMacros.h"`.
//
// Each macro forwards all arguments to Logger::Log{}, which uses CTAD to
// deduce template parameters and captures std::source_location at the call
// site via the constructor's default parameter.

#define LOG_TRACE(...) Logger::Log(Logger::LogLevel::Trace,   __VA_ARGS__)
#define LOG_DEBUG(...) Logger::Log(Logger::LogLevel::Debug,   __VA_ARGS__)
#define LOG_INFO(...)  Logger::Log(Logger::LogLevel::Info,    __VA_ARGS__)
#define LOG_WARN(...)  Logger::Log(Logger::LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) Logger::Log(Logger::LogLevel::Error,   __VA_ARGS__)
