module;

#include <cassert>
#include <Windows.h>

#include "GameDLLInterface.h"
#include "LoggerMacros.h"

module DllLoader;

#if defined ( __INTELLISENSE__ )
#include "DllLoader.ixx"
#include "EngineError.ixx"
#include "Logger.ixx"
#else
import Logger;
import Error;

import std;
#endif

namespace Engine
{
	Expected<void> CheckGameDllVersion(const DllLoader& dll)
	{
		if (dll.IsLoaded())
		{
			auto getVersion = dll.ResolveFunctionName<unsigned int(*)()>("GameDLL_GetVersion");
			if (getVersion.has_value())
			{
				if (getVersion.value()() == GAME_DLL_ABI_VERSION)
				{
					return Expected<void>();
				}

				return Engine::Unexpected{ Engine::Error { Engine::ErrorType::Dll, "Game Dll version mismatch!" } };
			}

			return Unexpected{ getVersion.error() };
		}

		return Engine::Unexpected{ Engine::Error { Engine::ErrorType::Dll, "Invalid Dll loader!"} };
	}

	DllLoader::~DllLoader()
	{
		Unload();
	}

	bool DllLoader::IsLoaded() const
	{
		return _moduleHandle != nullptr;
	}

	// "When specifying a path, be sure to use backslashes,
	// not forward slashes.
	Expected<void> DllLoader::Load(const std::wstring& path)
	{
		assert(_moduleHandle == nullptr && "DllLoader already loaded a dll.");

		if (_moduleHandle != nullptr)
		{
			return Engine::Unexpected{ Engine::Error { Engine::ErrorType::Dll, "Trying to reuse active Dll loader" } };
		}

		_moduleHandle = LoadLibraryW(path.data());
		if (_moduleHandle == nullptr)
		{
			LPVOID messageBuffer;
			DWORD lastError = GetLastError();
			DWORD errorMsgSuccess = FormatMessage
			(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,		// dwFlags
				nullptr,		// lpSource (source of error msg if not FORMAT_MESSAGE_FROM_SYSTEM)
				lastError,		// dwMessageId
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// dwLanguageID, literally the currently used language
				(LPTSTR)&messageBuffer, // lpBuffer, where the msg will go
				0,				// min # of chars to allocate
				nullptr			// arguments to insert into the msg
			);

			if (errorMsgSuccess == 0)
			{
				return Engine::Unexpected{ Engine::Error { Engine::ErrorType::Dll, "Unknown error." } };
			}

			const auto errorStringBuffer = (LPCTSTR)messageBuffer;
			const auto errorString = std::string(errorStringBuffer);

			LocalFree(messageBuffer);

			return Engine::Unexpected{ Engine::Error { Engine::ErrorType::Dll, errorString } };
		}

		return {};
	}

	bool DllLoader::Unload()
	{
		bool didUnload = false;

		if (_moduleHandle != nullptr)
		{
			didUnload = FreeLibrary(_moduleHandle);
			_moduleHandle = nullptr;
		}

		return didUnload;
	}
} // namespace Engine