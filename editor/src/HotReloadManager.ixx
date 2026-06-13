module;

#include <cassert>
#include <Windows.h>

#include "LoggerMacros.h"
#include "engine_core_api.h"

export module HotReloadManager;

import std;

#if defined ( __INTELLISENSE__ )
#include "../../engine-core/src/DllLoader.ixx"
#include "../../engine-core/src/EngineError.ixx"
#include "../../engine-core/src/FileWatcher.ixx"
#include "../../engine-core/src/Serialization.ixx"
#else
import DataFile;
import DllLoader;
import Error;
import FileWatcher;
import Serialization;
#endif

namespace Editor
{
	export enum class ReloadStatus
	{
		NoChange = 0,
		Succeeded = 1,
		Failed = 2
	};

	export class HotReloadManager
	{
	public:
		~HotReloadManager();
		Engine::Expected<void> LoadGameDLL(const std::wstring& path, const std::wstring& fileName);

	private:
		Engine::Expected<void> CopyDllToShadow(const std::wstring& dllPath, const std::wstring& filePath) const;
	};
} // namespace Editor