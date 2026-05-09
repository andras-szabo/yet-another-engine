#pragma once

// Convenience macros for the Logger module.
//
// C++ modules cannot export macros, so these live in a companion header.
// Usage: alongside `import Logger;`, add `#include "LoggerMacros.h"`.
//
// Each macro forwards all arguments to Logger::Log{}, which uses CTAD to
// deduce template parameters and captures std::source_location at the call
// site via the constructor's default parameter.

#define LOG_TRACE(...) Engine::Log(Engine::LogLevel::Trace,   __VA_ARGS__)
#define LOG_DEBUG(...) Engine::Log(Engine::LogLevel::Debug,   __VA_ARGS__)
#define LOG_INFO(...)  Engine::Log(Engine::LogLevel::Info,    __VA_ARGS__)
#define LOG_WARN(...)  Engine::Log(Engine::LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) Engine::Log(Engine::LogLevel::Error,   __VA_ARGS__)
