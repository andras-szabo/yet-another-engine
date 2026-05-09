module;

#include <expected>
#include <string>

#include "engine_core_api.h"

export module Error;

namespace Engine
{

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
		struct Error
	{
		ErrorType type;
		std::string message;

		ENGINE_CORE_API Error(ErrorType type_, const std::string& message_);
	};

	ENGINE_CORE_API Error::Error(ErrorType type_, const std::string& message_)
		: type{ type_ }, message{ message_ }
	{
	}

	export
		template<typename T>
	using Expected = std::expected<T, Error>;
}
