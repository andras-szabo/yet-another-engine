module;
#include <string>
#include <string_view>
#include "engine_core_api.h"

export module Utility;

namespace Engine
{
	export struct Version
	{
		int major{ 0 };
		int minor{ 0 };
		int patch{ 0 };

		ENGINE_CORE_API Version& operator=(std::string_view versionString);

		ENGINE_CORE_API bool operator==(const Version& other) const;
		ENGINE_CORE_API bool operator!=(const Version& other) const;
		ENGINE_CORE_API [[nodiscard]] std::string ToString() const;
	};

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
	export ENGINE_CORE_API std::string WideToUtf8(std::wstring_view wide);

	export ENGINE_CORE_API std::wstring MultiByteToWide(std::string_view multiByte);
} // namespace Engine
