module;

export module EditorTasks;

#if defined ( __INTELLISENSE__ )
#include <chrono>
#include <expected>
#include <future>
#include <mutex>
#include <string>
#include <thread>
#else
import std;
#endif

namespace Editor
{
	export class EditorTask
	{
	public:
		EditorTask() = default;
		EditorTask(const EditorTask& other) = delete;
		EditorTask& operator=(const EditorTask& other) = delete;

		EditorTask(std::future<std::expected<void, std::string>>&& future_);
		EditorTask(EditorTask&& other);
		EditorTask& operator=(EditorTask&& other);

		bool IsDone() const;
		float GetProgress() const;
		void SetProgress(float progress);

		std::expected<void, std::string> Result();

	private:
		std::future<std::expected<void, std::string>> _future;
		std::atomic<float> _progress;
	};
} // namespace Editor

module :private;

namespace Editor
{
	EditorTask::EditorTask(std::future<std::expected<void, std::string>>&& future_)
		: _future{ std::move(future_) }
	{
	}

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

	float EditorTask::GetProgress() const
	{
		return _progress;
	}

	void EditorTask::SetProgress(float p)
	{
		_progress = p;
	}
} // Editor