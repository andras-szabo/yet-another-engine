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

	export Expected<void> CheckGameDllVersion(const DllLoader& dll);
} // namespace Engine
