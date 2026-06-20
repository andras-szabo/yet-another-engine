module;

#include <random>

module GUID;

import std;

namespace Engine
{
	// The RNG lives in this module implementation unit (inside engine-core.dll)
	// so all GUID construction — whether called from the engine or a game DLL —
	// uses the same per-thread random state via the DLL's __declspec(dllimport).
	static thread_local std::random_device randomDevice;
	static thread_local std::mt19937_64 randomEngine(randomDevice());
	static thread_local std::uniform_int_distribution<unsigned long long> randomDistribution;

	GUID::GUID() : id{ randomDistribution(randomEngine) }
	{
	}

} // namespace Engine
