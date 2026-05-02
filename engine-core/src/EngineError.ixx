module;

#include <expected>
#include <string>

#include "engine_core_api.h"

export module EngineError;

export
enum class ErrorType
{
	Undefined = 0,
	LogicError = 1,
	NotFound = 2,
	OutOfBounds = 3,
	// ... TODO add more as needed
};

export
struct EngineError
{
	ErrorType type;
	std::string message;

	ENGINE_CORE_API EngineError(ErrorType type_, const std::string& message_);
};

ENGINE_CORE_API EngineError::EngineError(ErrorType type_, const std::string& message_)
	: type{ type_ }, message{ message_ }
{
}

export
template<typename T>
using Expected = std::expected<T, EngineError>;
