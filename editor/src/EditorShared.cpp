module;

module EditorShared;

#if defined ( __INTELLISENSE__ )
#include <string>
#include <unordered_map>

#include "EditorShared.ixx"
#else
import std;
#endif

namespace Editor
{
	void Context::CollectExecutorInfo(const std::unordered_map<std::string, CommandTaskFN>& executors)
	{
		editorCommands.clear();

		for (const auto& executor : executors)
		{
			editorCommands.insert(std::pair<std::string, std::string>
			{
				executor.first,
				"TBA" // TODO: add command description
			});
		}
	}
} // namespace Editor
