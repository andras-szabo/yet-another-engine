module;

#include <cassert>
#include <Windows.h>

#include "LoggerMacros.h"
#include "engine_core_api.h"

export module FileWatcher;

import std;

#if defined ( __INTELLISENSE__ )
#include "Logger.ixx"
#else
import Logger;
#endif

namespace Engine
{
	export class ENGINE_CORE_API FileWatcher
	{
	public:
		/// <summary>
		/// Open a FileWatcher which you can poll to check if the given file in the given
		/// directory has been modified. Example:
		/// Engine::FileWatcher watch(L"C:\\Foo\\Bar", L"baz.txt");
		/// </summary>
		/// <param name="directoryPath">The directory path; don't forget to escape backslashes.</param>
		/// <param name="targetFileName">The target file name.</param>
		FileWatcher(const std::wstring& directoryPath,
					std::wstring_view targetFileName);

		FileWatcher(const FileWatcher& other) = delete;
		FileWatcher& operator=(const FileWatcher& other) = delete;

		FileWatcher(FileWatcher&& other) = delete;
		FileWatcher& operator=(FileWatcher&& other) = delete;

		~FileWatcher();

		bool Poll();
		bool IsValid() const;
		bool IsAvailableToCopy() const;

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

module :private;

namespace Engine
{
	FileWatcher::FileWatcher(const std::wstring& directoryPath,
							 std::wstring_view targetFileName)
		: _watchedDirectoryPath{ directoryPath }, _watchedFileName { targetFileName }
	{
		LPCWSTR lpFileName { directoryPath.c_str() };
		DWORD dwDesiredAccess { FILE_LIST_DIRECTORY };
		DWORD dwShareMode { FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE };
		LPSECURITY_ATTRIBUTES lpSecurityAttributes { NULL };
		DWORD dwCreationDisposition { OPEN_EXISTING };
		DWORD dwFlagsAndAttributes { 
			FILE_ATTRIBUTE_NORMAL | 
			FILE_FLAG_BACKUP_SEMANTICS |	// required when opening directories
			FILE_FLAG_OVERLAPPED };
		HANDLE hTemplateFile { NULL };

		_handle = CreateFileW(
			lpFileName,
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDisposition,
			dwFlagsAndAttributes,
			hTemplateFile);

		if (_handle != INVALID_HANDLE_VALUE)
		{
			_changeBuffer = new DWORD[GetChangeBufferSize()];

			LPSECURITY_ATTRIBUTES lpEventAttributes{ NULL };
			BOOL bManualReset { false };
			BOOL bInitialState { false };
			LPCSTR lpName { NULL };

			_overlappedIO.hEvent = CreateEvent(
				lpEventAttributes,
				bManualReset,
				bInitialState,
				lpName
			);

			StartWatching();
		}
		else
		{
			LOG_WARNING("Invalid _handle. Error: {}", GetLastError());
		}
	}

	FileWatcher::~FileWatcher()
	{
		if (_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(_handle);
		}

		if (_overlappedIO.hEvent != NULL)
		{
			CloseHandle(_overlappedIO.hEvent);
		}

		delete[] _changeBuffer;
	}

	/// <summary>
	/// When ReadDirectoryChanges returns true (the dll was recompiled), it might be still
	/// adjusted by the linker, so we might have to wait a bit until it actually becomes
	/// available for copying.
	/// </summary>
	/// <returns></returns>
	bool FileWatcher::IsAvailableToCopy() const
	{
		if (!_isValid)
		{
			return false;
		}

		std::filesystem::path observedFile{ _watchedDirectoryPath };
		observedFile /= _watchedFileName;
		const auto permissions{ std::filesystem::status(observedFile).permissions()};
		return (permissions & std::filesystem::perms::others_read) != std::filesystem::perms::none;
	}

	bool FileWatcher::IsValid() const
	{
		return _isValid;
	}

	constexpr int FileWatcher::GetChangeBufferSize() const
	{
		return 1024;
	}

	void FileWatcher::StartWatching()
	{
		LPVOID lpBuffer{ _changeBuffer };
		DWORD nBufferLength{ sizeof(DWORD) * GetChangeBufferSize()};
		BOOL bWatchSubtree{ false };
		DWORD dwNotifyFilter{ FILE_NOTIFY_CHANGE_LAST_WRITE };
		LPDWORD lpBytesReturned{ NULL };
		LPOVERLAPPED lpOverlapped{ &_overlappedIO };
		LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine{ NULL };

		auto startWatchingSuccess = ReadDirectoryChangesW(
			_handle,
			lpBuffer,
			nBufferLength,
			bWatchSubtree,
			dwNotifyFilter,
			lpBytesReturned,
			lpOverlapped,
			lpCompletionRoutine);

		_isValid = startWatchingSuccess != NULL;
		
		assert(_isValid && "Invalid FileWatcher");
	}

	bool FileWatcher::Poll()
	{
		assert(_isValid && "Trying to poll invalid FileWatcher");

		DWORD numberOfBytesTransferred { 0 };
		BOOL bWait{ false };

		const auto getResultSuccess = GetOverlappedResult(
			_handle,
			&_overlappedIO,
			&numberOfBytesTransferred,
			bWait);

		bool fileDidChange { false };

		if (getResultSuccess)
		{
			if (numberOfBytesTransferred == 0)
			{
				// The change buffer overflowed; I think we should treat this
				// as a "yes, something potentially did change."
				fileDidChange = true;
			}

			auto evt = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(_changeBuffer); 
			while (true && !fileDidChange)
			{
				const DWORD name_len = evt->FileNameLength / sizeof(wchar_t);
				switch (evt->Action)
				{
					case FILE_ACTION_REMOVED:
					case FILE_ACTION_RENAMED_OLD_NAME:
					case FILE_ACTION_MODIFIED:
						if (std::wstring_view(evt->FileName, name_len) == _watchedFileName)
						{
							fileDidChange = true;
						}
						break;

					default:
						// TODO?
						break;
				}

				if (evt->NextEntryOffset)
				{
					uint8_t* nextEntry = reinterpret_cast<uint8_t*>(evt) + evt->NextEntryOffset;
					evt = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(nextEntry);
				}
				else
				{
					break;
				}
			}

			StartWatching();
		}

		return fileDidChange;
	}
} // namespace Engine