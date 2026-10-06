module;

#include <cassert>
#include <Windows.h>

#include "LoggerMacros.h"

module DirectoryWatcher;

#if defined ( __INTELLISENSE__ )
#include <filesystem>
#include "Utility.ixx"
#else
import std;
import Logger;
import Utility;
#endif

namespace Engine
{
	DirectoryWatcher::DirectoryWatcher()
	{
	}

	DirectoryWatcher::DirectoryWatcher(const std::wstring& directoryPath)
		: _watchedDirectoryPath { directoryPath }
	{
		Setup(directoryPath);
	}

	DirectoryWatcher::~DirectoryWatcher()
	{
		Teardown();
	}

	void DirectoryWatcher::Setup(const std::wstring& directoryPath)
	{
		_watchedDirectoryPath = directoryPath;
		LPCWSTR lpFileName{ directoryPath.c_str() };
		DWORD dwDesiredAccess{ FILE_LIST_DIRECTORY };
		DWORD dwShareMode{ FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE };
		LPSECURITY_ATTRIBUTES lpSecurityAttributes{ NULL };
		DWORD dwCreationDisposition{ OPEN_EXISTING };
		DWORD dwFlagsAndAttributes{
			FILE_ATTRIBUTE_NORMAL |
			FILE_FLAG_BACKUP_SEMANTICS |	// required when opening directories
			FILE_FLAG_OVERLAPPED };
		HANDLE hTemplateFile{ NULL };
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
			BOOL bManualReset{ FALSE };
			BOOL bInitialState{ FALSE };
			LPCSTR lpName{ NULL };
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
			LOG_WARNING("Failed to open directory watcher. Error: {}", GetLastError());
		}
	}

	void DirectoryWatcher::Teardown()
	{
		if (_changeBuffer)
		{
			delete[] _changeBuffer;
			_changeBuffer = nullptr;
		}

		if (_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(_handle);
			_handle = INVALID_HANDLE_VALUE;
		}

		if (_overlappedIO.hEvent)
		{
			CloseHandle(_overlappedIO.hEvent);
			_overlappedIO.hEvent = NULL;
		}

		_isValid = false;
	}

	void DirectoryWatcher::StartWatching()
	{
		LPVOID lpBuffer{ _changeBuffer };
		DWORD nBufferLength{ sizeof(DWORD) * GetChangeBufferSize() };
		BOOL bWatchSubtree{ true };
		DWORD dwNotifyFilter{ 
			FILE_NOTIFY_CHANGE_FILE_NAME		// this should pick up renaming, creating, or deleting a file
			| FILE_NOTIFY_CHANGE_LAST_WRITE		// this should pick up modifications to a file
		};

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
		assert(_isValid && "Invalid DirectoryWatcher");
	}

	constexpr int DirectoryWatcher::GetChangeBufferSize() const
	{
		return 4096;
	}

	bool DirectoryWatcher::IsValid() const
	{
		return _isValid;
	}

	bool DirectoryWatcher::Poll()
	{
		assert(_isValid && "Trying to poll invalid DirectoryWatcher");

		DWORD numberOfBytesTransferred{ 0 };
		BOOL bWait{ false };

		const auto getResultSuccess = GetOverlappedResult(
			_handle,
			&_overlappedIO,
			&numberOfBytesTransferred,
			bWait);

		if (getResultSuccess)
		{
			auto evt = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(_changeBuffer);
			while (evt)
			{
				const DWORD name_len = evt->FileNameLength / sizeof(wchar_t);
				const std::wstring fileNameW{ evt->FileName, name_len };
				const std::string fileName = Engine::WideToUtf8(fileNameW);

				switch (evt->Action)
				{
					case FILE_ACTION_ADDED:
						LOG_INFO("File added: {}", fileName);
						break;
					case FILE_ACTION_REMOVED:
						LOG_INFO("File removed: {}", fileName);
						break;
					case FILE_ACTION_MODIFIED:
						LOG_INFO("File modified: {}", fileName);
						break;
					case FILE_ACTION_RENAMED_OLD_NAME:
						LOG_INFO("File renamed (old name): {}", fileName);
						break;
					case FILE_ACTION_RENAMED_NEW_NAME:
						LOG_INFO("File renamed (new name): {}", fileName);
						break;
					default:
						//TODO?
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
			return true;
		}

		return false;
	}

} // namespace Engine