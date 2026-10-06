module;

#include <Windows.h>

module EditorShared;

#if defined ( __INTELLISENSE__ )
#include <iterator>
#include <string>
#include <thread>
#include <unordered_map>

#include "EditorShared.ixx"
#else
import std;
#endif

namespace Editor
{
	Context::~Context()
	{
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

	void Context::RequestQuit()
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		_state.isQuitRequested = true;
		SetEvent(_quitRequestedEventHandle);
	}

	bool Context::IsQuitRequested()
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		return _state.isQuitRequested;
	}

	void Context::CollectFileChangeEvent(const Engine::FileChangeEvent& event)
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		_fileChangeEventQueue.push_back(event);
	}

	ContextState Context::GetCurrentState()
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		return _state;
	}

	void Context::SetState(const ContextState& state)
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		_state = state;
	}

	void Context::SetProjectPath(std::wstring_view projectPath)
	{
		std::lock_guard<std::mutex> l{ _contextLock };
		_state.projectPath = std::wstring(projectPath);
	}
} // namespace Editor
