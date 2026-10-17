module;

#include <Windows.h>

export module EditorShared;

#if defined ( __INTELLISENSE__ )
#include <mutex>
#include <string>
#include <expected>
#include <functional>
#include <string>
#include <vector>
#include <sstream>
#include "../../engine-core/src/EngineError.ixx"
#include "../../engine-core/src/FileWatcher.ixx"
#include "../../engine-core/src/Utility.ixx"
#include "HotReloadManager.ixx"
#else
import std;
import Error;
import FileWatcher;
import HotReloadManager;
import Utility;
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

    export enum class EditorPlayState
    {
        Undefined = 0,

        Editing,
        StartingPlay,
        Playing,
        Paused,
        StoppingPlay,

        ShuttingDown
    };

    export struct EditorState
    {
        EditorPlayState currentState { EditorPlayState::Undefined };
		EditorPlayState desiredState { EditorPlayState::Undefined };

        std::string ToString() const
        {
            std::ostringstream oss;

			auto enumToStr = [](EditorPlayState state) -> std::string
				{
					switch (state)
					{
					case EditorPlayState::Undefined: return "Undefined";
					case EditorPlayState::Editing: return "Editing";
					case EditorPlayState::StartingPlay: return "StartingPlay";
					case EditorPlayState::Playing: return "Playing";
					case EditorPlayState::Paused: return "Paused";
					case EditorPlayState::StoppingPlay: return "StoppingPlay";
					case EditorPlayState::ShuttingDown: return "ShuttingDown";
					default: return "Unknown";
					}
				};

			oss << "current: " << enumToStr(currentState) << "; desired: " << enumToStr(desiredState);

            return oss.str();
        }

    };

    export struct ContextState
    {
        std::wstring gameTemplatePath{ L"" };
        std::wstring sdkPath{ L"" };
        std::wstring cmakePath{ L"" };
        std::wstring projectPath{ L"" };
        bool isQuitRequested{ false };
    };

    export struct ProjectSettings
    {
        std::string projectName{ "Untitled" };
        std::string dllPathDebug{ "" };
        std::string dllPathRelease{ "" };

        Engine::Version editorVersion{};
    };

    export struct Context
    {
        ~Context();

        std::unordered_map<std::string, std::string> editorCommands;
        void CollectExecutorInfo(const std::unordered_map<std::string, Editor::EditorCommand>& executors);
        void SetProjectPath(std::wstring_view projectPath);

        ContextState GetContextState();
        void SetContextState(const ContextState& state);

        bool IsQuitRequested();
        void RequestQuit();

        HANDLE GetQuitRequestedEventHandle();
        std::mutex& GetContextLock();

		void CollectFileChangeEvent(const Engine::FileChangeEvent& event);
        std::vector<Engine::FileChangeEvent> ConsumeFileChangeEvents();

        ProjectSettings GetProjectSettings();
		void SetProjectSettings(const ProjectSettings& settings);

		EditorState GetEditorState();
        void RequestEditorPlayModeStateChange(EditorPlayState requestedState);
        void ExecuteRequestedPlayModeStateChange();

        bool HasProjectDllChanged();
        Engine::Expected<void> ReloadProjectDll();

    private:
        std::mutex _contextLock;
        ContextState _contextState;
		HANDLE _quitRequestedEventHandle{ CreateEventW(NULL, TRUE, FALSE, NULL) };
        std::vector<Engine::FileChangeEvent> _fileChangeEventQueue;

        std::mutex _projectSettingsLock;
        ProjectSettings _projectSettings;

        std::mutex _editorStateLock;
        EditorState _editorState;

        std::mutex _hotreloadManagerLock;
        HotReloadManager _hotReloadManager;
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