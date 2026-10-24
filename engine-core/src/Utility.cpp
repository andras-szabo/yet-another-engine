module;

#include <Windows.h>

module Utility;

#if defined ( __INTELLISENSE__ )
#include <format>
#include <ranges>
#include <string>
#include <string_view>

#include "Utility.ixx"
#else
import std;
#endif

namespace Engine
{
	Version& Version::operator=(std::string_view versionString)
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

	bool Version::operator==(const Version& other) const
	{
		return major == other.major && minor == other.minor && patch == other.patch;
	}

	bool Version::operator!=(const Version& other) const
	{
		return !(*this == other);
	}

	[[nodiscard]] std::string Version::ToString() const
	{
		return std::format("{}.{}.{}", major, minor, patch);
	}

	std::wstring MultiByteToWide(std::string_view multiByte)
	{
		if (multiByte.empty())
		{
			return {};
		}
		
		const int sizeNeeded = MultiByteToWideChar(
			CP_UTF8, 0, multiByte.data(), static_cast<int>(multiByte.size()),
			nullptr, 0);

		std::wstring result(sizeNeeded, L'\0');
		MultiByteToWideChar(
			CP_UTF8, 0, multiByte.data(), static_cast<int>(multiByte.size()),
			result.data(), sizeNeeded);

		return result;
	}


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