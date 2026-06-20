module;

#include <Windows.h>
#include "engine_core_api.h"

export module FileWatcher;

#if defined ( __INTELLISENSE__ )
#include <string>
#else
import std;
#endif

namespace Engine
{
	export class FileWatcher
	{
	public:
		ENGINE_CORE_API FileWatcher();

		/// <summary>
		/// Open a FileWatcher which you can poll to check if the given file in the given
		/// directory has been modified. Example:
		/// Engine::FileWatcher watch(L"C:\\Foo\\Bar", L"baz.txt");
		/// </summary>
		/// <param name="directoryPath">The directory path; don't forget to escape backslashes.</param>
		/// <param name="targetFileName">The target file name.</param>
		ENGINE_CORE_API FileWatcher(const std::wstring& directoryPath,
									std::wstring_view targetFileName);

		FileWatcher(const FileWatcher& other) = delete;
		FileWatcher& operator=(const FileWatcher& other) = delete;

		FileWatcher(FileWatcher&& other) = delete;
		FileWatcher& operator=(FileWatcher&& other) = delete;

		ENGINE_CORE_API ~FileWatcher();

		ENGINE_CORE_API void Setup(const std::wstring& directoryPath, std::wstring_view targetFileName);
		ENGINE_CORE_API void Teardown();

		ENGINE_CORE_API bool Poll();
		ENGINE_CORE_API bool IsValid() const;
		ENGINE_CORE_API bool IsAvailableToCopy() const;

	private:
		HANDLE _handle { NULL };
		OVERLAPPED _overlappedIO {};
		DWORD* _changeBuffer { nullptr };
		bool _isValid{ false };
		std::wstring _watchedDirectoryPath;
		std::wstring _watchedFileName;

		void StartWatching();
		constexpr int GetChangeBufferSize() const;
	};
}// namespace Engine
