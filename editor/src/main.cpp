#include <iostream>
#include "LoggerMacros.h"

// Import the engine-core module.
// Once engine-core exports real symbols, they will be available here.
import EngineCore;
import Logger;

int main()
{
    //Logger::GlobalLoggerInstance.minLogLevel = Logger::LogLevel::Trace;

    //Logger::Log(Logger::LogLevel::Trace, "This is a trace");
    //Logger::Log(Logger::LogLevel::Debug, "This is a debug");
    //Logger::Log(Logger::LogLevel::Info, "This is an info");
    //Logger::Log(Logger::LogLevel::Warning, "This is a warning");
    //Logger::Log(Logger::LogLevel::Error, "This is an error");

    //Logger::Log(Logger::LogLevel::Info, "This is an info with params {} and {}", 42, "Crabalocker");
    //Logger::Log(Logger::LogLevel::Warning, "This is a warning with parameter {}", "wparam");
    //Logger::Log(Logger::LogLevel::Error, "This is an error with params {} and {} and {}", "foo", "bar", "baz");

    //Logger::GlobalLoggerInstance.minLogLevel = Logger::LogLevel::Warning;

    //Logger::Log(Logger::LogLevel::Info, "This should not appear.");
    //Logger::Log(Logger::LogLevel::Warning, "... but this should.");

    //std::cout << "[editor] stub - engine initialisation will go here\n";
    LOG_INFO("This is an info");
    return 0;
}
