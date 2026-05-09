module;

#include <format>
#include <functional>
#include <random>

#include "engine_core_api.h"

export module GUID;

export
struct ENGINE_CORE_API GUID
{
	constexpr static GUID Invalid();
	unsigned long long id;

	GUID();
	constexpr GUID(unsigned long long);
	constexpr GUID(const GUID& other) = default;
	constexpr GUID& operator=(const GUID& other) = default;
	constexpr GUID(GUID&& other) = default;
	constexpr GUID& operator=(GUID&& other) = default;

	constexpr bool operator==(const GUID& other) const;
	constexpr bool operator!=(const GUID& other) const;

	constexpr operator unsigned long long() const;
	constexpr bool IsValid() const;
};

namespace std
{
	template<>
	struct hash<GUID>
	{
		std::size_t operator()(const GUID& guid) const
		{
			return hash<unsigned long long>()(guid.id);
		}
	};
}

static std::random_device randomDevice;
static std::mt19937_64 randomEngine(randomDevice());
static std::uniform_int_distribution<unsigned long long> randomDistribution;

GUID::GUID() : id{ randomDistribution(randomEngine) }
{}

constexpr GUID::GUID(unsigned long long id_) : id{ id_ }
{}

constexpr GUID GUID::Invalid()
{
	return GUID(0);
}

constexpr GUID::operator unsigned long long() const
{
	return id;
}

constexpr bool GUID::operator==(const GUID& other) const
{
	return id == other.id;
}

constexpr bool GUID::operator!=(const GUID& other) const
{
	return id != other.id;
}

constexpr bool GUID::IsValid() const
{
	return id != 0;
}

template<>
struct std::formatter<GUID>
{
private:
	mutable std::formatter<unsigned long long> _ulongFormatter;

public:
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return _ulongFormatter.parse(ctx);
	}

	auto format(GUID guid, std::format_context& ctx) const
	{
		auto out = ctx.out();
		out = _ulongFormatter.format(guid.id, ctx);
		return out;
	}
};



