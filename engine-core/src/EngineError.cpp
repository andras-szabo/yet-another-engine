module;

#include <string>

#include "engine_core_api.h"

module Error;

import std;

namespace Engine
{
	Error::Error(ErrorType type_, const std::string& message_)
		: type{ type_ }, message{ message_ }
	{
	}

} // namespace Engine
