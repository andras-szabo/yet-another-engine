module;
#include <string>
#include <string_view>
#include <Windows.h>
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

	// Converts a UTF-16 (wchar_t) string to a UTF-8 encoded std::string.
	// Use this instead of truncating casts like std::string{w.begin(), w.end()},
	// which silently corrupt any non-ASCII characters.
	export ENGINE_CORE_API
	std::string WideToUtf8(std::wstring_view wide)
	{
		if (wide.empty())
		{
			return {};
		}

		const int sizeNeeded = WideCharToMultiByte(
			CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
			nullptr, 0, nullptr, nullptr);

		std::string result(sizeNeeded, '\0');
		WideCharToMultiByte(
			CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
			result.data(), sizeNeeded, nullptr, nullptr);

		return result;
	}
} // namespace Engine
