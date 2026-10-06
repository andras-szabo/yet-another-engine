module;

#include <Windows.h>

export module EditorShared;

#if defined ( __INTELLISENSE__ )
#include <mutex>
#include <string>
#include <expected>
#include <functional>
#include <span>
#include <vector>
#include "../../engine-core/src/FileWatcher.ixx"
#else
import std;
import FileWatcher;
#endif

namespace Editor
{
    struct Context;
    struct IEditorTask;

    export typedef std::expected<void, std::string> CommandReturnType;
    export typedef std::function<CommandReturnType(const std::vector<std::string>&, Context&, IEditorTask&)> CommandTaskFN;

    export struct EditorCommand
    {
        CommandTaskFN commandFunction;
        std::string description;
    };

    export struct ContextState
    {
        std::wstring gameTemplatePath{ L"" };
        std::wstring sdkPath{ L"" };
        std::wstring cmakePath{ L"" };
        std::wstring projectPath{ L"" };
        bool isQuitRequested{ false };
    };

    export struct Context
    {
        ~Context();

        std::unordered_map<std::string, std::string> editorCommands;
        void CollectExecutorInfo(const std::unordered_map<std::string, Editor::EditorCommand>& executors);
        void SetProjectPath(std::wstring_view projectPath);

        ContextState GetCurrentState();
        void SetState(const ContextState& state);

        bool IsQuitRequested();
        void RequestQuit();

        HANDLE GetQuitRequestedEventHandle();
        std::mutex& GetContextLock();

		void CollectFileChangeEvent(const Engine::FileChangeEvent& event);
        std::vector<Engine::FileChangeEvent> ConsumeFileChangeEvents();

    private:
        std::mutex _contextLock;
        ContextState _state;
		HANDLE _quitRequestedEventHandle{ CreateEventW(NULL, TRUE, FALSE, NULL) };
        std::vector<Engine::FileChangeEvent> _fileChangeEventQueue;
    };

    export struct IEditorTask
    {
        ~IEditorTask() = default;

        virtual bool IsDone() const = 0;
        virtual float GetProgress() = 0;
        virtual void SetProgress(float p) = 0;
    };

    export struct TaskProgressScope
    {
        TaskProgressScope(IEditorTask& task, float start = 0.0f, float end = 1.0f);
        ~TaskProgressScope();

    private:
        IEditorTask& _task;
        float _start{ 0.0f };
        float _end{ 1.0f };
        
    };


} // namespace Editor

module :private;

namespace Editor
{
    TaskProgressScope::TaskProgressScope(IEditorTask& task, float start, float end)
        : _task{ task }, _start{ start }, _end{ end }
    {
        _task.SetProgress(_start);
    }

    TaskProgressScope::~TaskProgressScope()
    {
        _task.SetProgress(_end);
    }
} //namespace Editor