module;

#include <Windows.h>
#include "LoggerMacros.h"

module EditorShared;

#if defined ( __INTELLISENSE__ )
#include <iterator>
#include <string>
#include <thread>
#include <unordered_map>

#include "EditorShared.ixx"
#include "../../engine-core/src/DirectoryWatcher.ixx"
#include "../../engine-core/src/Logger.ixx"
#else
import DirectoryWatcher;
import FileWatcher;
import Logger;
import std;
#endif

namespace Editor
{
	Context::~Context()
	{
		if (_directoryWatcherThread.joinable())
		{
			_directoryWatcherThread.join();
		}

		if (_quitRequestedEventHandle)
		{
			CloseHandle(_quitRequestedEventHandle);
		}

		_quitRequestedEventHandle = nullptr;
	}

	void Context::CollectExecutorInfo(const std::unordered_map<std::string, EditorCommand>& executors)
	{
		editorCommands.clear();

		for (const auto& executor : executors)
		{
			editorCommands.insert(std::pair<std::string, std::string>
			{
				executor.first,
				executor.second.description
			});
		}
	}

	std::vector<Engine::FileChangeEvent> Context::ConsumeFileChangeEvents()
	{
		std::vector<Engine::FileChangeEvent> events;
		{
			std::lock_guard<std::mutex> guard{ _contextLock };
			if (!_fileChangeEventQueue.empty())
			{
				events = std::move(_fileChangeEventQueue);
			}
		}

		return events;
	}

	HANDLE Context::GetQuitRequestedEventHandle()
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		return _quitRequestedEventHandle;
	}

	HANDLE Context::GetProjectDirectoryWatcherStopEventHandle()
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		return _projectDirectoryWatcherStopEventHandle;
	}

	void Context::RequestQuit()
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		_contextState.isQuitRequested = true;
		SetEvent(_quitRequestedEventHandle);
	}

	bool Context::IsQuitRequested()
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		return _contextState.isQuitRequested;
	}

	bool Context::HasProjectOpen()
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		return !_contextState.projectPath.empty();
	}

	void Context::CollectFileChangeEvent(const Engine::FileChangeEvent& event)
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		_fileChangeEventQueue.push_back(event);
	}

	ContextState Context::GetContextState()
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		return _contextState;
	}

	void Context::SetContextState(const ContextState& state)
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		_contextState = state;
	}

	void Context::SetProjectPath(std::string_view projectPath)
	{
		std::lock_guard<std::mutex> l{ _contextLock };
		_contextState.projectPath = std::string(projectPath);
	}

	ProjectSettings Context::GetProjectSettings()
	{
		std::lock_guard<std::mutex> l{ _projectSettingsLock };
		return _projectSettings;
	}

	EditorState Context::GetEditorState()
	{
		std::lock_guard<std::mutex> l{ _editorStateLock };
		return _editorState;
	}

	void Context::CloseProject()
	{
		const auto unloadDllResult = UnloadProjectDll();
		if (!unloadDllResult.has_value())
		{
			LOG_ERROR(unloadDllResult.error().message);
		}

		StopProjectDirectoryWatcher();

		std::lock_guard<std::mutex> l{ _contextLock };
		_contextState.projectPath.clear();
		SetProjectSettings({});
	}

	bool Context::HasProjectDllChanged()
	{
		std::lock_guard<std::mutex> l{ _hotreloadManagerLock };
		return _hotReloadManager.HasDllChanged();
	}

	Engine::Expected<void> Context::UnloadProjectDll()
	{
		std::lock_guard<std::mutex> l{ _hotreloadManagerLock };
		return _hotReloadManager.UnloadGameDll();
	}

	Engine::Expected<void> Context::ReloadProjectDll()
	{
		std::lock_guard<std::mutex> l{ _hotreloadManagerLock };
		return _hotReloadManager.ReloadDll();
	}

	Engine::Expected<void> Context::LoadProjectDll()
	{
		std::lock_guard<std::mutex> l{ _hotreloadManagerLock };

		std::filesystem::path dllFolderPath { _contextState.projectPath };
		dllFolderPath /= _projectSettings.DllFolderPath();

		//const std::wstring fileName = Engine::MultiByteToWide(_projectSettings.DllFileName());

		std::println("Loading project DLL from folder: {}, file name: {}", dllFolderPath.string(), _projectSettings.DllFileName());

		return _hotReloadManager.LoadGameDll(dllFolderPath.string(), _projectSettings.DllFileName());
	}

	void Context::RequestEditorPlayModeStateChange(EditorPlayState requestedState)
	{
		std::lock_guard<std::mutex> l{ _editorStateLock };
		_editorState.desiredState = requestedState;
	}

	void Context::ExecuteRequestedPlayModeStateChange()
	{
		std::lock_guard<std::mutex> l{ _editorStateLock };
		_editorState.currentState = _editorState.desiredState;
	}

	void Context::SetProjectSettings(const ProjectSettings& settings)
	{
		std::lock_guard<std::mutex> l{ _projectSettingsLock };
		_projectSettings = settings;
	}

	void Context::StartProjectDirectoryWatcher()
	{
		StopProjectDirectoryWatcher();

		std::lock_guard<std::mutex> l{ _editorStateLock };
		_directoryWatcherThread = std::thread(PollDirectoryChanges, std::ref(*this));
	}

	void Context::StopProjectDirectoryWatcher()
	{
		SetEvent(_projectDirectoryWatcherStopEventHandle);

		if (_directoryWatcherThread.joinable())
		{
			_directoryWatcherThread.join();
		}

		ResetEvent(_projectDirectoryWatcherStopEventHandle);
	}

	void Context::PollDirectoryChanges(Context& context)
	{
		LOG_INFO("Starting directory watcher thread...");

		// Poll the context to see if a project path has been set; it not, wait for it.
		bool hasProjectPath = false;
		while (!hasProjectPath)
		{
			const auto state = context.GetContextState();
			if (!state.projectPath.empty())
			{
				hasProjectPath = true;
			}
			else
			{
				if (state.isQuitRequested)
				{
					LOG_INFO("Quit requested; exiting directory watcher thread.");
					return;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
		}

		LOG_INFO("Project path set; starting directory watcher for {}", context.GetContextState().projectPath);

		auto watcher = Engine::DirectoryWatcher{ context.GetContextState().projectPath };
		std::function<void(const Engine::FileChangeEvent&)> callback = [&context](const Engine::FileChangeEvent& event)
			{
				context.CollectFileChangeEvent(event);
			};

		// The red squiggly line here is Intellisense getting a bit confused about modules.
		watcher.RegisterCallback(callback);

		bool keepWaiting{ true };
		while (keepWaiting)
		{
			HANDLE handles[3] = {
				watcher.GetWaitHandle(),
				context.GetQuitRequestedEventHandle(),
				context.GetProjectDirectoryWatcherStopEventHandle()
			};

			DWORD waitForChangesOrQuit = WaitForMultipleObjects(
				3,
				handles,
				FALSE,
				INFINITE);

			if (waitForChangesOrQuit == WAIT_OBJECT_0)
			{
				watcher.Poll();
			}
			else
			{
				if (waitForChangesOrQuit != WAIT_OBJECT_0 + 1 &&
					waitForChangesOrQuit != WAIT_OBJECT_0 + 2)
				{
					LOG_ERROR("Unexpected result from WaitForMultipleObjects: {}", waitForChangesOrQuit);
				}

				keepWaiting = false;
			}
		}
	}

} // namespace Editor
