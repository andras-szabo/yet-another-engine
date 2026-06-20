module;

#include <cassert>
#include <Windows.h>

#include "LoggerMacros.h"

module FileWatcher;

#if defined ( __INTELLISENSE__ )
#include <filesystem>
#else
import Logger;
import std;
#endif

namespace Engine
{
	FileWatcher::FileWatcher()
	{
	}

	FileWatcher::FileWatcher(const std::wstring& directoryPath,
		std::wstring_view targetFileName)
		: _watchedDirectoryPath{ directoryPath }, _watchedFileName{ targetFileName }
	{
		Setup(directoryPath, targetFileName);
	}

	void FileWatcher::Setup(const std::wstring& directoryPath,
		std::wstring_view targetFileName)
	{
		_watchedDirectoryPath = directoryPath;
		_watchedFileName = targetFileName;

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
			BOOL bManualReset{ false };
			BOOL bInitialState{ false };
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
			LOG_WARNING("Invalid _handle. Error: {}", GetLastError());
		}
	}

	void FileWatcher::Teardown()
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

		_handle = INVALID_HANDLE_VALUE;
		_overlappedIO.hEvent = NULL;
		_changeBuffer = nullptr;
		_isValid = false;
	}

	FileWatcher::~FileWatcher()
	{
		Teardown();
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
		const std::wstring pathAsWString = observedFile.wstring();

		LPCWSTR lpFileName{ pathAsWString.c_str() };
		DWORD dwDesiredAccess{ GENERIC_READ };
		DWORD dwShareMode{ 0 };
		LPSECURITY_ATTRIBUTES lpSecurityAttributes{ NULL };
		DWORD dwCreationDisposition{ OPEN_EXISTING };
		DWORD dwFlagsAndAttributes{ FILE_ATTRIBUTE_NORMAL };
		HANDLE hTemplateFile{ NULL };

		const auto handle = CreateFileW(
			lpFileName,
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDisposition,
			dwFlagsAndAttributes,
			hTemplateFile);

		if (handle == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		CloseHandle(handle);
		return true;

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
		DWORD nBufferLength{ sizeof(DWORD) * GetChangeBufferSize() };
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

		DWORD numberOfBytesTransferred{ 0 };
		BOOL bWait{ false };

		const auto getResultSuccess = GetOverlappedResult(
			_handle,
			&_overlappedIO,
			&numberOfBytesTransferred,
			bWait);

		bool fileDidChange{ false };

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