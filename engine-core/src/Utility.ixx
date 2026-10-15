module;
#include <string>
#include <string_view>
#include <Windows.h>
#include "engine_core_api.h"
#include <format>
#include <ranges>

export module Utility;

namespace Engine
{
	export struct Version
	{
		int major{ 0 };
		int minor{ 0 };
		int patch{ 0 };

		ENGINE_CORE_API
		Version& operator=(std::string_view versionString)
		{
			auto parts = std::views::split(versionString, '.');
			int index = 0;
			for (const auto part : parts)
			{
				if (index == 0)
				{
					major = std::stoi(std::string{ part.begin(), part.end() });
				}
				else if (index == 1)
				{
					minor = std::stoi(std::string{ part.begin(), part.end() });
				}
				else if (index == 2)
				{
					patch = std::stoi(std::string{ part.begin(), part.end() });
				}
				else
				{
					break;
				}

				++index;
			}

			return *this;
		}

		ENGINE_CORE_API
		bool operator==(const Version& other) const
		{
			return major == other.major && minor == other.minor && patch == other.patch;
		}

		ENGINE_CORE_API
		bool operator!=(const Version& other) const
		{
			return !(*this == other);
		}

		ENGINE_CORE_API
		[[nodiscard]] std::string ToString() const
		{
			return std::format("{}.{}.{}", major, minor, patch);
		}
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
