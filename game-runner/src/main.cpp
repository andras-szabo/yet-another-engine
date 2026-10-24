#include <iostream>
#include "LoggerMacros.h"

#if defined ( __INTELLISENSE__ )

#include "../../engine-core/src/DllLoader.ixx"
#include "../../engine-core/src/EngineCore.ixx"
#include "../../engine-core/src/EngineInstance.ixx"

#else
import DllLoader;
import EngineCore;
import EngineInstance;

import std;
#endif

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		LOG_ERROR("Usage: {} <path_to_game_dll>", argv[0]);
		return 1;
	}

	const std::string gameDllPath{ argv[1] };
	LOG_INFO("Game dll path: {}", gameDllPath);

	Engine::DllLoader dllLoader;
	auto loadResult = dllLoader.Load(gameDllPath);

	if (!loadResult.has_value())
	{
		LOG_ERROR("Failed to load game dll: {}", loadResult.error().message);
		return 1;
	}

	LOG_INFO("Successfully loaded game dll: {}", gameDllPath);

	if (Engine::CheckGameDllVersion(dllLoader).has_value())
	{
		LOG_INFO("Game dll version is compatible.");
	}
	else
	{
		LOG_ERROR("Game dll version is not compatible.");
		return 1;
	}

	return 0;
}
