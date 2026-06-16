module;

#include <cassert>
#include <Windows.h>

#include "LoggerMacros.h"
#include "engine_core_api.h"

export module HotReloadManager;

import std;

#if defined ( __INTELLISENSE__ )
#include "../../engine-core/src/DataFile.ixx"
#include "../../engine-core/src/DllLoader.ixx"
#include "../../engine-core/src/EngineError.ixx"
#include "../../engine-core/src/EngineInstance.ixx"
#include "../../engine-core/src/FileWatcher.ixx"
#include "../../engine-core/src/Logger.ixx"
#include "../../engine-core/src/Serialization.ixx"
#else
import DataFile;
import DllLoader;
import EngineInstance;
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
		Engine::Expected<void> LoadGameDll(const std::wstring& dllFolderPath, 
			const std::wstring& fileName);

		bool IsDllLoaded() const;

	private:
		Engine::Expected<std::filesystem::path> CopyDllToShadow(const std::wstring& dllFolderPath, const std::wstring& fileName) const;
		Engine::Expected<Engine::DataFile> SerializeCurrentScene() const;

		Engine::DllLoader _loader{};
	};
} // namespace Editor

module :private;

namespace Editor
{
	namespace fs = std::filesystem;

	bool HotReloadManager::IsDllLoaded() const
	{
		return _loader.IsLoaded();
	}

	Engine::Expected<Engine::DataFile> HotReloadManager::SerializeCurrentScene() const
	{
		Engine::DataFile fOut;
		try
		{
			Engine::SerializeScene(Engine::Instance.GetActiveScene(), fOut);
		}
		catch (std::runtime_error& e)
		{
			return Engine::Unexpected{ Engine::Error { Engine::ErrorType::Serialization, e.what() } };
		}

		return fOut;
	}

	Engine::Expected<void> HotReloadManager::LoadGameDll(const std::wstring& dllFolderPath,
		const std::wstring& fileName)
	{
		Engine::DataFile previousScene;

		if (IsDllLoaded())
		{
			const auto serializedScene = SerializeCurrentScene();
			if (!serializedScene.has_value())
			{
				return Engine::Unexpected{ Engine::Error{ serializedScene.error() } };
			}

			previousScene = serializedScene.value();

			Engine::Instance.GetActiveScene().Clear();
			const bool didUnload = _loader.Unload();

			if (!didUnload)
			{
				return Engine::Unexpected{ Engine::Error { Engine::ErrorType::Dll, "Couldn't unload game dll." } };
			}
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

		if (!previousScene.IsEmpty())
		{
			auto newScene = Engine::DeserializeScene(previousScene, Engine::Instance.GetComponentStorage());
			if (!newScene.has_value())
			{
				return Engine::Unexpected{ Engine::Error {Engine::ErrorType::Deserialization, "Couldn't load previous scene." } };
			}

			Engine::Instance.GetActiveScene() = std::move(newScene.value());
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