module;

export module EditorTasks;

#if defined ( __INTELLISENSE__ )
#include <chrono>
#include <expected>
#include <future>
#include <mutex>
#include <string>
#include <thread>
#include "EditorShared.ixx"
#else
import EditorShared;
import std;
#endif

namespace Editor
{
	export class EditorTask : public IEditorTask
	{
	public:
		EditorTask() = default;
		EditorTask(const EditorTask& other) = delete;
		EditorTask& operator=(const EditorTask& other) = delete;

		EditorTask(CommandTaskFN commandFN,
				   const std::vector<std::string>& commandAndArguments,
			       Context& editorContext,
				   bool async = true);

		EditorTask(EditorTask&& other);
		EditorTask& operator=(EditorTask&& other);

		bool IsDone() const override;
		float GetProgress() override;
		void SetProgress(float progress) override;

		std::expected<void, std::string> Result();

	private:
		std::future<std::expected<void, std::string>> _future;
		std::atomic<float> _progress;
	};
} // namespace Editor

module :private;

namespace Editor
{
	EditorTask::EditorTask(EditorTask&& other)
		: _future{ std::move(other._future) }
	{
		_progress.store(other._progress);
	}

	EditorTask& EditorTask::operator=(EditorTask&& other)
	{
		if (&other != this)
		{
			_future = std::move(other._future);
			_progress.store(other._progress);
		}
		return *this;
	}

	EditorTask::EditorTask(CommandTaskFN commandFN,
		const std::vector<std::string>& commandAndArguments,
		Context& editorContext,
		bool async)
	{
		if (async)
		{
			_future = std::async(std::launch::async,
				commandFN,
				std::cref(commandAndArguments),
				std::ref(editorContext),
				std::ref(*this));
		}
		else
		{
			std::promise<std::expected<void, std::string>> promise;
			_future = promise.get_future();

			try
			{
				promise.set_value(commandFN(
					std::cref(commandAndArguments),
					std::ref(editorContext),
					std::ref(*this)
				));
			}
			catch (...)
			{
				promise.set_exception(std::current_exception());
			}
		}
	}

	std::expected<void, std::string> EditorTask::Result()
	{
		return _future.get();
	}

	bool EditorTask::IsDone() const
	{
		using namespace std::chrono_literals;
		const auto status = _future.wait_for(0s);
		return status == std::future_status::ready;
	}

	float EditorTask::GetProgress()
	{
		return _progress;
	}

	void EditorTask::SetProgress(float p)
	{
		_progress = p;
	}
} // Editor