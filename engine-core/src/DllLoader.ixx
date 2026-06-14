module;

#include <cassert>
#include <Windows.h>	// TODO - platform layer so we can swap it easily

#include "GameDLLInterface.h"
#include "LoggerMacros.h"
#include "engine_core_api.h"

export module DllLoader;

#if defined ( __INTELLISENSE__ )
#include "EngineError.ixx"
#include "Logger.ixx"
#else
import std;
import Error;
import Logger;
#endif

namespace Engine
{
	export class ENGINE_CORE_API DllLoader
	{
	public:
		DllLoader() = default;
		~DllLoader();

		Expected<void> Load(const std::wstring& path);
		bool Unload();
		bool IsLoaded() const;

		/// <summary>
		/// Usage example: loader.Resolve<unsigned int(*)()>("GameDLL_GetVersion")
		/// </summary>
		/// <typeparam name="FPtr"></typeparam>
		/// <param name="name"></param>
		/// <returns></returns>
		template<typename FPtr>
		Expected<FPtr> ResolveFunctionName(const char* name) const;

	private:
		HMODULE _moduleHandle { nullptr };
	};

	template <typename FPtr>
	Expected<FPtr> DllLoader::ResolveFunctionName(const char* name) const
	{
		assert(_moduleHandle != nullptr && "Trying to resolve function on invalid DllLoader");

		FARPROC address = GetProcAddress(_moduleHandle, name);
		if (address == NULL)
		{
			return Unexpected{ Error { ErrorType::NotFound, name } };
		}

		return reinterpret_cast<FPtr>(address);
	}

	export Expected<void> CheckGameDllVersion(const DllLoader& dll)
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

				return Unexpected{ Error { ErrorType::Dll, "Game Dll version mismatch!" } };
			}

			return Unexpected{ getVersion.error() };
		}

		return Unexpected{ Error { ErrorType::Dll, "Invalid Dll loader!"} };
	}

} // namespace Engine

module :private;

namespace Engine
{
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
			return Unexpected{ Error { ErrorType::Dll, "Trying to reuse active Dll loader" } };
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
				(LPTSTR) &messageBuffer, // lpBuffer, where the msg will go
				0,				// min # of chars to allocate
				nullptr			// arguments to insert into the msg
			);

			if (errorMsgSuccess == 0)
			{
				return Unexpected{ Error { ErrorType::Dll, "Unknown error." } };
			}

			const auto errorStringBuffer = (LPCTSTR)messageBuffer;
			const auto errorString = std::string(errorStringBuffer);

			LocalFree(messageBuffer);

			return Unexpected{ Error { ErrorType::Dll, errorString } };
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
