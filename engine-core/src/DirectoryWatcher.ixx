module;

#include <Windows.h>
#include "engine_core_api.h"

export module DirectoryWatcher;

#if defined ( __INTELLISENSE__ )
#include <string>
#else
import std;
#endif

namespace Engine
{
	export class DirectoryWatcher
	{
	public:
		ENGINE_CORE_API DirectoryWatcher();

		/// <summary>
		/// Open a DirectoryWatcher which you can poll to check if any file in the given directory
		/// has been modified.
		/// </summary>
		/// <param name="directoryPath">The directory path; don't forget to escape backslashes.</param>
		/// <returns></returns>
		ENGINE_CORE_API DirectoryWatcher(const std::wstring& directoryPath);

		DirectoryWatcher(const DirectoryWatcher& other) = delete;
		DirectoryWatcher& operator=(const DirectoryWatcher& other) = delete;

		DirectoryWatcher(DirectoryWatcher&& other) = delete;
		DirectoryWatcher& operator=(DirectoryWatcher&& other) = delete;

		ENGINE_CORE_API ~DirectoryWatcher();
		ENGINE_CORE_API void Setup(const std::wstring& directoryPath);
		ENGINE_CORE_API void Teardown();

		ENGINE_CORE_API bool IsValid() const;
		ENGINE_CORE_API bool Poll();

		// TODO: This is a bit unfortunate, as this breaks encapsulation by
		// exposing the internal OVERLAPPED event to callers. 
		ENGINE_CORE_API HANDLE GetWaitHandle() const;

	private:
		HANDLE _handle{ NULL };
		OVERLAPPED _overlappedIO{};
		DWORD* _changeBuffer{ nullptr };
		bool _isValid{ false };
		std::wstring _watchedDirectoryPath;

		void StartWatching();
		constexpr int GetChangeBufferSize() const;

	};
}