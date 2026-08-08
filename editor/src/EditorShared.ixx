module;

export module EditorShared;

#if defined ( __INTELLISENSE__ )
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

    export struct Context
    {
        std::wstring gameTemplatePath{ L"" };
        std::wstring sdkPath{ L"" };
        std::wstring cmakePath{ L"" };
        bool isQuitRequested{ false };

        std::unordered_map<std::string, std::string> editorCommands;

        void CollectExecutorInfo(const std::unordered_map<std::string, Editor::CommandTaskFN>& executors);
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