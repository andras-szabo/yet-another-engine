module;

#include <string>
#include "engine_core_api.h"

export module Utility;

namespace Engine
{
	export ENGINE_CORE_API
	constexpr unsigned int DJBHash(const std::string& str)
	{
		unsigned int hash = 5381;
		for (char c : str)
		{
			hash = ((hash << 5) + hash) + c;
		}
		return hash;
	}
} // namespace Engine
