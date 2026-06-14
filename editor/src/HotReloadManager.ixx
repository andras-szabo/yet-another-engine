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
#include "../../engine-core/src/Logger.ixx"
#include "../../engine-core/src/Serialization.ixx"
#else
import DataFile;
import DllLoader;
import Error;
import FileWatcher;
import Logger;
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
		HotReloadManager();
		~HotReloadManager();

		Engine::Expected<void> LoadGameDll(const std::wstring& dllFolderPath, 
			const std::wstring& fileName);

		bool IsDllLoaded() const;

	private:
		Engine::Expected<std::filesystem::path> CopyDllToShadow(const std::wstring& dllFolderPath, 
			const std::wstring& fileName) const;

		Engine::DllLoader _loader{};
	};
} // namespace Editor

module :private;

namespace Editor
{
	namespace fs = std::filesystem;

	// TODO cleanup
	HotReloadManager::HotReloadManager()
	{
		//const std::string temp = (std::string) fs::temp_directory_path().string();
		//LOG_INFO("[Foo] HotReloadManager ctor. Temp path: {}", temp);
	}

	// TODO cleanup
	HotReloadManager::~HotReloadManager()
	{
		//LOG_INFO("[Foo] HotReloadManager dtor.");
	}

	bool HotReloadManager::IsDllLoaded() const
	{
		return _loader.IsLoaded();
	}

	Engine::Expected<void> HotReloadManager::LoadGameDll(const std::wstring& dllFolderPath,
		const std::wstring& fileName)
	{
		if (IsDllLoaded())
		{
			// Serialize scene
			// Tear it down
			// Clear component storage
			// Dll unload
			throw std::runtime_error{ "Scene teardown not yet implemented." };
		}

		const auto shadowDllPath = CopyDllToShadow(dllFolderPath, fileName);
		if (!shadowDllPath.has_value())
		{
			return Engine::Unexpected{ Engine::Error { shadowDllPath.error() } };
		}

		_loader.Load(shadowDllPath.value());

		if (!_loader.IsLoaded())
		{
			return Engine::Unexpected{ Engine::Error { Engine::ErrorType::Dll, "Failed to load DLL shadow copy." } };
		}

		return {};
	}

	Engine::Expected<fs::path> HotReloadManager::CopyDllToShadow(const std::wstring& dllFolderPath,
		const std::wstring& fileName) const
	{
		fs::path sourcePath{ dllFolderPath };
		sourcePath /= fileName;

		if (!fs::exists(sourcePath))
		{
			return Engine::Unexpected{ Engine::Error { Engine::ErrorType::File, "Game DLL not found."} };
		}

		fs::path tempPath { fs::temp_directory_path() };
		const auto shadowDllName = std::format(L"shadow_{}", fileName);
		tempPath /= shadowDllName;

		const auto copyOptions = fs::copy_options::overwrite_existing;
		std::error_code error;

		if (fs::copy_file(sourcePath, tempPath, copyOptions, error) == false)
		{
			return Engine::Unexpected{ Engine::Error { Engine::ErrorType::File, std::format("Game DLL copy error: {}", error.message())} };
		}

		return tempPath;
	}
} // namespace Editor