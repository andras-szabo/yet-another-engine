module;

module EditorShared;

#if defined ( __INTELLISENSE__ )
#include <string>
#include <thread>
#include <unordered_map>

#include "EditorShared.ixx"
#else
import std;
#endif

namespace Editor
{
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

	void Context::RequestQuit()
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		_state.isQuitRequested = true;
	}

	bool Context::IsQuitRequested()
	{
		std::lock_guard<std::mutex> guard{ _contextLock };
		return _state.isQuitRequested;
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
