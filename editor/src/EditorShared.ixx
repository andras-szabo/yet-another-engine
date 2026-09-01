module;

export module EditorShared;

#if defined ( __INTELLISENSE__ )
#include <mutex>
#include <string>
#else
#endif

import std;

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
        std::unordered_map<std::string, std::string> editorCommands;
        void CollectExecutorInfo(const std::unordered_map<std::string, Editor::EditorCommand>& executors);
        void SetProjectPath(std::wstring_view projectPath);

        ContextState GetCurrentState();
        void SetState(const ContextState& state);

        bool IsQuitRequested();
        void RequestQuit();

    private:
        std::mutex _contextLock;
        ContextState _state;
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