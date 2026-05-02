#include <iostream>

// Import the engine-core module.
// Once engine-core exports real symbols, they will be available here.
import EngineCore;
import Logger;

int main()
{
    Logger::Log(Logger::LogLevel::Info, "This is an info");
    Logger::Log(Logger::LogLevel::Warning, "This is a warning");
    Logger::Log(Logger::LogLevel::Error, "This is an error");

    Logger::Log(Logger::LogLevel::Info, "This is an info with params {} and {}", 42, "Crabalocker");
    Logger::Log(Logger::LogLevel::Warning, "This is a warning with parameter {}", "wparam");
    Logger::Log(Logger::LogLevel::Error, "This is an error with params {} and {} and {}", "foo", "bar", "baz");

    //std::cout << "[editor] stub - engine initialisation will go here\n";
    return 0;
}
