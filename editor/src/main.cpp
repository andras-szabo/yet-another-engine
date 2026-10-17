#include <iostream>
#include <cassert>
#include <filesystem>
#include <functional>
#include <Windows.h>
#include <string>
#include <vector>
#include "LoggerMacros.h"

// Import the engine-core module.
// Once engine-core exports real symbols, they will be available here.
#if defined ( __INTELLISENSE__ )
#include "../../engine-core/src/DirectoryWatcher.ixx"
#include "../../engine-core/src/EngineError.ixx"
#include "../../engine-core/src/EngineInstance.ixx"
#include "../../engine-core/src/FileWatcher.ixx"
#include "../../engine-core/src/Logger.ixx"
#include "../../engine-core/src/Math.ixx"
#include "../../engine-core/src/Reflection.ixx"
#include "../../engine-core/src/Scene.ixx"
#include "../../engine-core/src/Serialization.ixx"
#include "EditorTests.ixx"
#include "HotReloadManager.ixx"
#include "EditorTasks.ixx"
#include "EditorShared.ixx"
#else
import EditorShared;
import EngineCore;
import EngineInstance;
import EditorTests;
import FileWatcher;
import HotReloadManager;
import EditorTasks;
import std;
#endif

using namespace Engine;

Engine::Expected<int> ShouldBeEven(int number)
{
	if (number % 2 == 0)
	{
		return number;
	}

	return std::unexpected<Engine::Error>({ Engine::ErrorType::Logic, "Bloargh" });
}

void TestM3x3()
{
	Engine::Vec3 a{ 1, 2, 3 };
	Engine::Vec3 b = Engine::Mat3x3::Identity() * a;
	LOG_INFO("[Foo] identity * a = {}", b);
}

void TestVec4()
{
	Engine::Vec4 a;
	Engine::Vec4 b{ 1, 2, 3, 0 };
	Engine::Vec4 c{ 5, 6, 7, 8 };
	LOG_INFO("[Foo] Vec4: a: {}, b: {}, b + c = {}", a, b, b + c);

	Engine::Vec4 d;
	Engine::Vec4 e{ 10, 9, 8, 7 };
	LOG_INFO("[Foo] Lerp 0.25: {} // 0.75: {}", Lerp(d, e, 0.25f), Lerp(d, e, 0.75f));
}

void TestVec3()
{
	Engine::Vec3 a;
	Engine::Vec3 b{ 1, 2.23f, 3 };
	Engine::Vec3 c = a + b;
	LOG_INFO("[Foo] Vec3: a: {}, b: {}, a + b = {:.2f}", a, b, c);
}

void TestVec2()
{
	constexpr Engine::Vec2 a{};
	static_assert(a.x == 0.0f && a.y == 0.0f);

	constexpr Engine::Vec2 b{ 1, 2 };
	static_assert(b.x == 1.0f && b.y == 2.0f);

	Engine::Vec2 d = Engine::Vec2::Right();
	d += Engine::Vec2::Left();
	d -= Engine::Vec2::Right();

	LOG_INFO("[Foo] right + left - right = {}", d);

	Engine::Vec2 e = Engine::Vec2::Up() * 12.3456f;
	LOG_INFO("[Foo] Up * 12.3456f = {:.2f}", e);

	LOG_INFO("[Foo] 'e' normalized = {}", e.Normalized());

	Engine::Vec2 f{ 10, 10 };
	Engine::Vec2 g{ 1, 0 };
	LOG_INFO("[Foo] 'f.g' = {}", f.Dot(g));

	Engine::Vec2 h{ 123, 456 };
	LOG_INFO("[Foo] Before: {}, {}", h[0], h[1]);

	h[0] = 999.0f;
	h[1] = -42.0f;
	LOG_INFO("[Foo] After: {}", h);

	LOG_INFO("Vec2 tests run.");
}

void TestDF()
{
	LOG_INFO("Starting datafile test ---");

	Engine::DataFile df;

	df["Foo"] = 1234.f;
	df["Bar"] = 42;
	df["Baz"] = "Baz";

	Engine::Vec2 v{ -123.4f, 3.14f };
	df["Position"].SetFloats(2, v.x, v.y);

	Engine::Vec2 vCopy{ df["Position"].GetFloat(0), df["Position"].GetFloat(1) };

	LOG_INFO("Df with assignment ops: {}", df.ToString());
	LOG_INFO("Df deserialized: {}", vCopy);
}

struct NestedType
{
	int john{ 0 };
	float paul{ 0.f };
	Engine::Vec3 george;
	Engine::Quaternion ringo;
};

Engine::FieldSpan GetNestedTypeFieldDescriptors()
{
	static Engine::FieldDescriptor nt_john{ "john", Engine::FieldType::Int, offsetof(NestedType, john) };
	static Engine::FieldDescriptor nt_paul{ "paul", Engine::FieldType::Float, offsetof(NestedType, paul) };
	static Engine::FieldDescriptor nt_george{ "george", Engine::FieldType::Vec3, offsetof(NestedType, george) };
	static Engine::FieldDescriptor nt_ringo{ "ringo", Engine::FieldType::Quaternion, offsetof(NestedType, ringo) };

	static std::array<Engine::FieldDescriptor, 4> fields
	{
		nt_john, nt_paul, nt_george, nt_ringo
	};

	return Engine::FieldSpan{ fields.data(), 4 };
}

void TestSrsly()
{
	struct TestType
	{
		int foo;
		int bar;
		float baz;
		NestedType fizz;
	};

	Engine::FieldDescriptor f_foo{ "foo", Engine::FieldType::Int, offsetof(TestType, foo) };
	Engine::FieldDescriptor f_bar{ "bar", Engine::FieldType::Int, offsetof(TestType, bar) };
	Engine::FieldDescriptor f_baz{ "baz", Engine::FieldType::Float, offsetof(TestType, baz) };
	Engine::FieldDescriptor f_fizz{ "fizz", Engine::FieldType::Composite, offsetof(TestType, fizz), GetNestedTypeFieldDescriptors };

	std::array<Engine::FieldDescriptor, 4> fields
	{
		f_foo, f_bar, f_baz, f_fizz
	};

	TestType t;
	t.foo = 123;
	t.bar = -234;
	t.baz = 45.424f;
	t.fizz.john = 99;
	t.fizz.paul = -42.f;
	t.fizz.george = { -5.0f, -6.0f, -7.0f };
	t.fizz.ringo = Engine::Quaternion::Identity();

	Engine::DataFile df{};

	/*Engine::SerializeFields(&t, fields, df);
	LOG_INFO("Serialization at work:\n{}", df.ToString());

	LOG_INFO("And now to deserialize...");
	TestType read;
	Engine::DeserializeFields(&read, fields, df);
	LOG_INFO("Deserialized: foo = {}, bar = {}, baz = {}", read.foo, read.bar, read.baz);
	*/
}

void RunTests()
{
	EditorTests::RunSceneTest();
}

void RunFileWatcherTest()
{
	Engine::FileWatcher watcher(L"C:\\Users\\andra", L"todo.txt");

	if (watcher.IsValid())
	{
		LOG_INFO("FileWatcher is valid.");

		int waitForSeconds = 30;

		while (waitForSeconds-- > 0)
		{
			if (watcher.Poll())
			{
				LOG_INFO("Yup, the file was just modified.");
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

	}
	else
	{
		LOG_WARNING("FileWatcher is not valid.");
	}
}

int tests()
{
	LOG_INFO("This is an info");
	auto bar = ShouldBeEven(42);
	auto baz = ShouldBeEven(3);

	if (bar.has_value())
	{
		LOG_INFO("OK, bar checks out!, and its value is {}", bar.value());
	}

	if (!baz.has_value())
	{
		LOG_ERROR("Oho. {}", baz.error().message);
	}

	TestVec2();
	TestVec3();
	TestVec4();
	TestM3x3();
	TestDF();
	TestSrsly();

	for (int i = 0; i < 5; ++i)
	{
		LOG_INFO("Creating a guid: {}", Engine::GUID{});
	}

	Engine::EngineInstance::Initialize(std::make_unique<Engine::ComponentStorage>());

	LOG_INFO("-----");
	RunTests();
	LOG_INFO("-----");

	auto g = Engine::EngineInstance::GetActiveSceneRW()
		.CreateGameObject(&Engine::EngineInstance::GetComponentStorage(), "Foo", 0);

	g->GetName();

	assert(g->GetTransform() != nullptr && "GameObject created but has no transform.");
	LOG_INFO("G has a transform, yay!");
	LOG_INFO("Active scene: {}", Engine::EngineInstance::GetActiveScene().GetSceneName());

	// Let's walk through it...
	auto logNodes = [](Engine::Scene::Scene& scene, std::size_t currentIndex) {
		LOG_INFO("Index: {}, name: {}", currentIndex, scene.GetNodeName(currentIndex));
		};

	auto& scene = Engine::EngineInstance::GetActiveSceneRW();

	scene.WalkBreadthFirst(0, logNodes);

	auto firstChild = scene.CreateGameObject(&Engine::EngineInstance::GetComponentStorage(),
		"FirstChild", g->GetTransform()->GetSceneNodeIndex());

	LOG_INFO(scene.GetNodeName(firstChild->GetTransform()->GetSceneNodeIndex()));

	// Add some nodes to the root
	for (int i = 0; i < 3; ++i)
	{
		scene.CreateGameObject(&Engine::EngineInstance::GetComponentStorage(),
			std::format("child_{}", i),
			g->GetTransform()->GetSceneNodeIndex());
	}

	scene.WalkBreadthFirst(0, logNodes);

	// Add a nested hierarchy to the first child
	int parentIndex = firstChild->GetTransform()->GetSceneNodeIndex();
	auto cs = &Engine::EngineInstance::GetComponentStorage();
	for (int i = 0; i < 3; ++i)
	{
		const std::string name = std::format("nested_child_{}", i);
		auto newParent = scene.CreateGameObject(cs, name, parentIndex);
		parentIndex = newParent->GetTransform()->GetSceneNodeIndex();
	}

	LOG_INFO("--- \n\n ---");

	LOG_INFO("Depth-first walk");
	scene.WalkDepthFirst(0, logNodes);

	LOG_INFO("Breadth-first walk");
	scene.WalkBreadthFirst(0, logNodes);

	scene.UpdateWorldTransforms();

	LOG_INFO("World transforms updated");

	Engine::DataFile fOut;
	Engine::SerializeScene(scene, fOut);

	LOG_INFO("Serialized scene: {}", fOut.ToString());

	// And now,, the deserialization... o.O
	auto sceneMaybe = Engine::DeserializeScene(fOut, Engine::EngineInstance::GetComponentStorage());
	if (!sceneMaybe.has_value())
	{
		LOG_ERROR("Uh-oh. {}", sceneMaybe.error().message);
	}
	else
	{
		LOG_INFO("My word. We deserialized a scene! Let's try to log it! \n\n");
		auto& newScene = sceneMaybe.value();
		LOG_INFO("Break here...");
		newScene.WalkDepthFirst(0, logNodes);

		LOG_INFO("Tearing down active scene");
		Engine::EngineInstance::GetActiveSceneRW().Clear();

		LOG_INFO("... and replacing it");
		Engine::EngineInstance::SetActiveScene(std::move(newScene));
		Engine::EngineInstance::GetActiveSceneRW().WalkDepthFirst(0, logNodes);
	}

	Editor::HotReloadManager hrm;
	LOG_WARNING("This is just a mock warning.");

	//RunFileWatcherTest();

	// Try to add another transform; this should fail.
	//g.AddComponent<Engine::Transform>();

	/*Engine::Transform* addedComponent = g.AddComponent<Engine::Transform>();
	assert(addedComponent != nullptr && "Couldn't add transform");

	LOG_INFO("Added trsf, and its value is {}", (std::size_t)addedComponent);

	Engine::Transform* gotComponent = g.GetComponent<Engine::Transform>();
	assert(addedComponent == gotComponent && "Couldn't get transform");

	LOG_INFO("Yay we could get trsf, and its value is {}", (std::size_t) gotComponent);*/

	return 0;
}

// TODO - handle spaces:
//      track an inQuotes toggle on ", only split on whitespace
//      when not inside quotes; strip the quote chars themselves.
//      then, expected use: createProject "C:\My Project"
std::vector<std::string> Split(const std::string& str)
{
	std::vector<std::string> tokens;
	std::istringstream iss{ str };
	std::string token;
	while (iss >> token)
	{
		tokens.push_back(token);
	}
	return tokens;
}

Editor::CommandReturnType ListAssets([[maybe_unused]] const std::vector<std::string>& commandAndArguments,
	[[maybe_unused]] Editor::Context& context,
	Editor::IEditorTask& task)
{
	task.SetProgress(0.0f);
	const auto& assetDatabase = Engine::EngineInstance::GetAssetDatabase();
	const auto assets = assetDatabase.GetAssetGUIDsByPath();
	std::cout << "Assets:\n";

	const auto count = static_cast<float>(assets.size());
	float currentCount = 0.0f;

	for (const auto& pathGuidPair : assets)
	{
		std::cout << "  " << pathGuidPair.first << " -> " << pathGuidPair.second.id << "\n";
		currentCount += 1.0f;
		task.SetProgress(currentCount / count);
	}

	task.SetProgress(1.0f);
	return {};
}

Editor::CommandReturnType PopulateAssetDatabase(
	[[maybe_unused]] const std::vector<std::string>& commandAndArguments,
	[[maybe_unused]] Editor::Context& context,
	Editor::IEditorTask& task)
{
	task.SetProgress(0.0f);

	auto& assetDatabase = Engine::EngineInstance::GetAssetDatabase();

	const auto contextState = context.GetContextState();
	const auto populateResult = assetDatabase.PopulateFromFolder(contextState.projectPath);
	if (!populateResult.has_value())
	{
		return std::unexpected{ "Failed to populate asset database: " + populateResult.error().message };
	}

	task.SetProgress(1.0f);
	return {};
}

Editor::CommandReturnType Help([[maybe_unused]] const std::vector<std::string>& commandAndArguments,
	[[maybe_unused]] Editor::Context& context,
	Editor::IEditorTask& task)
{
	std::cout << std::this_thread::get_id() << "\n";
	task.SetProgress(0.0f);
	const float totalCmdCount = static_cast<float>(context.editorCommands.size());
	float cmdIndex = 1.0f;
	std::cout << "Available commands: \n";

	std::vector<std::string> keys;
	for (const auto& cmd : context.editorCommands)
	{
		keys.push_back(cmd.first);
	}

	std::sort(keys.begin(), keys.end());

	for (const auto& key : keys)
	{
		const auto& cmd = context.editorCommands[key];
		std::cout << key << "   -   " << cmd << "\n";
		task.SetProgress(cmdIndex / totalCmdCount);
		cmdIndex += 1.0f;
	}

	return {};
}


Editor::CommandReturnType Cls([[maybe_unused]] const std::vector<std::string>& commandAndArguments,
	[[maybe_unused]] Editor::Context& context,
	Editor::IEditorTask& task)
{
	system("cls");
	task.SetProgress(1.0f);
	return {};
}

Editor::CommandReturnType Quit([[maybe_unused]] const std::vector<std::string>& commandAndArguments,
	Editor::Context& context,
	Editor::IEditorTask& task)
{
	context.RequestQuit();
	task.SetProgress(1.0f);
	return {};
}

Editor::CommandReturnType Status([[maybe_unused]] const std::vector<std::string>& commandAndArguments,
	Editor::Context& context,
	Editor::IEditorTask& task)
{
	std::cout << "Editor context:\n";
	task.SetProgress(0.5f);

	const auto contextState = context.GetContextState();
	const auto editorState = context.GetEditorState();

	std::cout << "  Quit requested? " << contextState.isQuitRequested << "\n";
	std::cout << "  Editor play state: " << editorState.ToString() << "\n";
	std::wcout << "  Game template path: " << contextState.gameTemplatePath << "\n";
	std::wcout << "  SDK path: " << contextState.sdkPath << "\n";
	std::wcout << "  CMake path: " << contextState.cmakePath << "\n";
	std::wcout << "  Project path: " << contextState.projectPath << "\n";

	const auto projectSettings = context.GetProjectSettings();
	std::cout << "  Project settings:\n";
	std::cout << "    Project Name: " << projectSettings.projectName << "\n";
	std::cout << "    Editor Version: " << projectSettings.editorVersion.ToString() << "\n";
	std::cout << "    DLL Path Debug: " << projectSettings.dllPathDebug << "\n";
	std::cout << "    DLL Path Release: " << projectSettings.dllPathRelease << "\n";

	task.SetProgress(1.0f);

	return {};
}

Editor::CommandReturnType OpenProject(const std::vector<std::string>& commandAndArguments,
	Editor::Context& context,
	Editor::IEditorTask& task)
{
	namespace fs = std::filesystem;
	Editor::TaskProgressScope _(task);
	if (commandAndArguments.size() < 2)
	{
		return std::unexpected{ "Missing path." };
	}

	fs::path projectFolderPath{ commandAndArguments[1] };
	if (!fs::exists(projectFolderPath))
	{
		return std::unexpected{ "Path does not exist." };
	}
	context.SetProjectPath(projectFolderPath.wstring());

	// 1. Look for the project settings file 
	// TODO: extract to its own function

	fs::path projectSettingsFilePath{ projectFolderPath };
	projectSettingsFilePath /= "project.udf";

	if (!fs::exists(projectSettingsFilePath))
	{
		LOG_INFO("Project settings file not found, creating default one.");
		// Create a default project settings file
		Engine::DataFile defaultSettings;

		defaultSettings["ProjectName"] = "Untitled";

		// TODO- this should come from the editor instance
		defaultSettings["EditorVersion"] = "0.0.1";

		defaultSettings["DLLPathDebug"] = "build\\Debug\\Game.dll";
		defaultSettings["DLLPathRelease"] = "build\\Release\\Game.dll";

		if (!Engine::DataFile::Serialize(defaultSettings, projectSettingsFilePath.string()))
		{
			return std::unexpected{ "Failed to create project settings file." };
		}
	}

	LOG_INFO("Project settings file found, loading settings.");
	const auto settingsMaybe = Engine::DataFile::Deserialize(projectSettingsFilePath.string());
	if (settingsMaybe.has_value())
	{
		const auto& settings = settingsMaybe.value();
		std::cout << "Project Name: " << settings["ProjectName"].GetString() << "\n";
		std::cout << "Editor Version: " << settings["EditorVersion"].GetString() << "\n";
		std::cout << "DLL Path Debug: " << settings["DLLPathDebug"].GetString() << "\n";
		std::cout << "DLL Path Release: " << settings["DLLPathRelease"].GetString() << "\n";

		Editor::ProjectSettings projectSettings;

		projectSettings.projectName = settings["ProjectName"].GetString();
		projectSettings.editorVersion = settings["EditorVersion"].GetString();
		projectSettings.dllPathDebug = settings["DLLPathDebug"].GetString();
		projectSettings.dllPathRelease = settings["DLLPathRelease"].GetString();

		context.SetProjectSettings(projectSettings);
	}
	else
	{
		return std::unexpected{ "Failed to load project settings." };
	}

	// And now we should:
	//  1. Look for the project settings file (some specific asset that needs to be in the project root).
	//      = in the project folder, look for a specific asset file ("project.udf" - unalmas data file?)
	//          = if the file is not there, create with default values:
	//                  = project name "Untitled",
	//                  = editor version
	//                  = dll path debug "build\Debug\Game.dll"
	//                  = dll path release "build\Release\Game.dll"
	//          = once the file is opened, extract settings from it,
	//              and create some shared project settings object that the editor can use,
	//              maybe similar to context state but different-?
	//    
	//
	//      = extract project name 
	//      = extract dll path
	//  2. Look for the serialized asset DB file.
	//      = this should contain a list of assets that we actually want to open in the project,
	//        on a folder-by-folder basis, and some kind of hash that describes the state of each folder.
	//      = we can then collect folder hashes and see if any of them have changed since,
	//        and if so, we can re-populate the asset database for that folder.
	//      (Or, for now, we can just re-populate the entire asset db from the project root, and
	//      then we can optimize later. )
	//  3. Load the game dll and wait for reloads.

	return {};
}

Editor::CommandReturnType CreateProject(const std::vector<std::string>& commandAndArguments,
	Editor::Context& context,
	Editor::IEditorTask& task)
{
	namespace fs = std::filesystem;

	Editor::TaskProgressScope _(task);

	if (commandAndArguments.size() < 2)
	{
		return std::unexpected{ "Missing path." };
	}

	fs::path projectFolderPath{ commandAndArguments[1] };

	if (fs::exists(projectFolderPath))
	{
		return std::unexpected{ "Path already exists." };
	}

	std::cout << "OK, trying to create folder at " << projectFolderPath.string() << "...\n";

	std::error_code errorCode;
	if (!fs::create_directory(projectFolderPath, errorCode))
	{
		return std::unexpected{ std::format("Couldn't create folder: {}", errorCode.message()) };
	}

	std::cout << "OK, folder created, now to copy files...\n";

	const auto contextState = context.GetContextState();

	// Copy "src" folder
	fs::path gameTemplateFolderPath{ contextState.gameTemplatePath };

	fs::copy_options options{ fs::copy_options::recursive };

	fs::path src{ gameTemplateFolderPath };
	src /= "src";

	fs::path target_src{ projectFolderPath };
	target_src /= "src";

	fs::copy(src, target_src, options, errorCode);

	if (errorCode)
	{
		return std::unexpected{ errorCode.message() };
	}

	// Copy "CMakeLists.txt"
	fs::path src_cmakeLists{ gameTemplateFolderPath };
	src_cmakeLists /= "CMakeLists.txt";

	fs::copy(src_cmakeLists, projectFolderPath, fs::copy_options::none, errorCode);
	if (errorCode)
	{
		return std::unexpected{ errorCode.message() };
	}

	std::cout << "...done!\n";
	std::cout << "Now generating project...\n";

	const std::wstring cmakePathWithSpaces = std::format(L"\"{}\"", contextState.cmakePath);
	std::wstring cmdLine{ cmakePathWithSpaces + L" -B build -G \"Visual Studio 17 2022\" -DENGINE_CORE_SDK_DIR=" + contextState.sdkPath };

	STARTUPINFOW startupInfo{};
	startupInfo.cb = sizeof(startupInfo);

	PROCESS_INFORMATION processInfo{};

	const auto workingPath = projectFolderPath.wstring();

	if (!CreateProcessW(
		contextState.cmakePath.data(),
		cmdLine.data(),
		nullptr,
		nullptr,
		FALSE,
		0,
		nullptr,
		projectFolderPath.wstring().c_str(),
		&startupInfo,
		&processInfo))
	{
		return std::unexpected{ std::format("Project generation failed: {}", GetLastError()) };
	}

	WaitForSingleObject(processInfo.hProcess, INFINITE);
	DWORD exitCode = 0;
	GetExitCodeProcess(processInfo.hProcess, &exitCode);
	CloseHandle(processInfo.hThread);
	CloseHandle(processInfo.hProcess);

	if (exitCode != 0)
	{
		return std::unexpected{ std::format("Project generation failed; exited with {}", exitCode) };
	}

	std::cout << "...Done!\n";

	context.SetProjectPath(projectFolderPath.wstring());

	return {};
}

Editor::CommandReturnType ProjectName(const std::vector<std::string>& commandAndArguments,
	Editor::Context& context,
	Editor::IEditorTask& task)
{
	Editor::TaskProgressScope _(task);

	const bool readOnly = commandAndArguments.size() == 1;

	auto projectSettings = context.GetProjectSettings();

	if (readOnly)
	{
		std::cout << "Project name: " << projectSettings.projectName << "\n";
		return {};
	}

	// Concatenate the remaining arguments to form the project name, in case it contains spaces.
	std::ostringstream projectNameStream;

	for (int i = 1; i < commandAndArguments.size(); ++i)
	{
		if (i > 1)
		{
			projectNameStream << " ";
		}

		projectNameStream << commandAndArguments[i];
	}

	projectSettings.projectName = projectNameStream.str();

	context.SetProjectSettings(projectSettings);
	const auto projectPath = context.GetContextState().projectPath;

	if (projectPath.empty())
	{
		return std::unexpected{ "No project path set." };
	}

	std::filesystem::path projectSettingsFilePath{ projectPath };
	projectSettingsFilePath /= "project.udf";
	Engine::DataFile df;
	df["ProjectName"] = projectSettings.projectName;
	df["EditorVersion"] = projectSettings.editorVersion.ToString();
	df["DLLPathDebug"] = projectSettings.dllPathDebug;
	df["DLLPathRelease"] = projectSettings.dllPathRelease;
	if (!Engine::DataFile::Serialize(df, projectSettingsFilePath.string()))
	{
		return std::unexpected{ "Failed to save project settings file." };
	}

	std::cout << "Project name set to: " << projectSettings.projectName << "\n";

	return {};
}

Editor::CommandReturnType Echo(const std::vector<std::string>& commandAndArguments,
	[[maybe_unused]] Editor::Context& context,
	Editor::IEditorTask& task)
{
	Editor::TaskProgressScope _(task);
	if (commandAndArguments.size() > 1 && commandAndArguments[1] == "banana")
	{
		return std::unexpected{ "nope i won't echo that.\n" };
	}

	int index = 0;
	for (const auto& arg : commandAndArguments)
	{
		if (index++ > 0)
		{
			std::cout << arg << " ";
		}
	}

	std::cout << "\n";

	return {};
}

Editor::CommandReturnType PausePlayMode([[maybe_unused]]const std::vector<std::string>& commandAndArguments,
	Editor::Context& context,
	Editor::IEditorTask& task)
{
	Editor::TaskProgressScope _(task);
	auto editorState = context.GetEditorState();

	if (editorState.currentState == Editor::EditorPlayState::Playing)
	{
		context.RequestEditorPlayModeStateChange(Editor::EditorPlayState::Paused);
	}
	else if (editorState.currentState == Editor::EditorPlayState::Paused)
	{
		context.RequestEditorPlayModeStateChange(Editor::EditorPlayState::Playing);
	}

	return {};
}

Editor::CommandReturnType TogglePlayMode([[maybe_unused]]const std::vector<std::string>& commandAndArguments,
	Editor::Context& context,
	Editor::IEditorTask& task)
{
	Editor::TaskProgressScope _(task);
	auto editorState = context.GetEditorState();

	if (editorState.currentState == Editor::EditorPlayState::Editing)
	{
		context.RequestEditorPlayModeStateChange(Editor::EditorPlayState::StartingPlay);

	}
	else if (editorState.currentState == Editor::EditorPlayState::Playing || editorState.currentState == Editor::EditorPlayState::Paused)
	{
		context.RequestEditorPlayModeStateChange(Editor::EditorPlayState::StoppingPlay);
	}

	return {};
}

Editor::CommandReturnType ListSceneNodes([[maybe_unused]] const std::vector<std::string>& commandAndArguments,
	[[maybe_unused]] Editor::Context& context,
	Editor::IEditorTask& task)
{
	Editor::TaskProgressScope _(task);
	const auto& scene = Engine::EngineInstance::GetActiveScene();
	std::cout << "Scene: " << scene.GetSceneName() << "\n";
	std::cout << "Nodes:\n";
	const auto nodeCount = scene.GetTransformStorage()->transformComponents.size();
	float currentNodeIndex = 0.0f;
	for (std::size_t i = 0; i < nodeCount; ++i)
	{
		std::cout << "  " << scene.GetNodeName(i) << "\n";
		currentNodeIndex += 1.0f;
		task.SetProgress(currentNodeIndex / static_cast<float>(nodeCount));
	}
	return {};
}


void TryExecute(const std::vector<std::string>& tokens,
	std::unordered_map<std::string, Editor::EditorCommand>& executors,
	Editor::Context& context)
{
	std::cout << "======== (" << std::this_thread::get_id() << ")\n";
	if (tokens.size() > 0)
	{
		const auto command = tokens[0];
		auto executor = executors.find(command);
		if (executor != executors.end())
		{
			const auto& fn = executor->second.commandFunction;
			Editor::EditorTask et(fn, tokens, context);

			// We could poll the task, but for now:

			const auto ret = et.Result();

			std::cout << "========\n";

			if (ret.has_value())
			{
				std::cout << "Command executed! Progress: " << et.GetProgress() << "\n";
			}
			else
			{
				std::cout << ret.error() << " " << et.GetProgress() << "\n";
			}
		}
	}
}

std::expected<std::wstring, std::wstring> GetVSBundledCmakePath()
{
	// Since VS 2017+, every VS installer places vswhere.exe at a fixed, edition-independent
	// path: C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe.
	// Run it once (e.g. vshwere -latest -property installationPath) to get the VS install
	// root, then append the fixed suffix:
	// Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
	// This is the standard, MS-documented way tools locate VS-bundled binaries without
	// hardcoding an edition / path.
	// https://learn.microsoft.com/en-us/visualstudio/install/tools-for-managing-visual-studio-instances?view=visualstudio

	SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, TRUE };
	HANDLE readPipe{ nullptr };
	HANDLE writePipe{ nullptr };
	if (!CreatePipe(&readPipe, &writePipe, &sa, 0))
	{
		return std::unexpected{ L"Couldn't create pipe." };
	}

	SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFOW si{};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = writePipe;
	si.hStdError = writePipe;
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

	PROCESS_INFORMATION pi{};
	std::wstring cmdLine = L"C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere -latest -property installationPath";

	BOOL ok = CreateProcessW(
		nullptr,                // LPCWSTR lpApplicationName, can be null
		cmdLine.data(),         // LPWSTR lpCommandLine, the command line to be executed
		nullptr, nullptr,       // security and thread attributes; can be null
		TRUE,                   // bInheritHandles
		CREATE_NO_WINDOW,       // dwCreationFlags
		nullptr,                // lpEnvironment; if null, uses the env of the calling process
		nullptr,                // lpCurrentDirectory, can be null
		&si,                    // startupInfo
		&pi);                   // processInfo

	CloseHandle(writePipe);

	if (!ok)
	{
		CloseHandle(readPipe);
		const auto errorCode = GetLastError();
		return std::unexpected{ std::format(L"Process invocation error: {}", errorCode) };
	}

	std::wstring output;
	char buffer[4096];
	DWORD bytesRead = 0;

	while (ReadFile(readPipe, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead)
	{
		int wideLen = MultiByteToWideChar(CP_UTF8, 0, buffer, (int)bytesRead, nullptr, 0);
		if (wideLen > 0)
		{
			const std::size_t oldSize{ output.size() };
			output.resize(oldSize + wideLen);
			MultiByteToWideChar(CP_UTF8, 0, buffer, (int)bytesRead, &output[oldSize], wideLen);
		}
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(readPipe);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	while (!output.empty() && (output.back() == L'\r' || output.back() == L'\n'))
	{
		output.pop_back();
	}

	output += L"\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake.exe";

	return output;
}

void PollDirectoryChanges(Editor::Context& context)
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

	const auto pathAsUtf8 = Engine::WideToUtf8(context.GetContextState().projectPath);
	LOG_INFO("Project path set; starting directory watcher for {}", pathAsUtf8);

	auto watcher = Engine::DirectoryWatcher{ context.GetContextState().projectPath };

	watcher.RegisterCallback([&context](const Engine::FileChangeEvent& event)
		{
			context.CollectFileChangeEvent(event);
		});

	bool keepWaiting{ true };
	while (keepWaiting)
	{
		HANDLE handles[2] = {
			watcher.GetWaitHandle(),
			context.GetQuitRequestedEventHandle()
		};

		DWORD waitForChangesOrQuit = WaitForMultipleObjects(
			2,
			handles,
			FALSE,
			INFINITE);

		if (waitForChangesOrQuit == WAIT_OBJECT_0)
		{
			watcher.Poll();
		}
		else
		{
			if (waitForChangesOrQuit != WAIT_OBJECT_0 + 1)
			{
				LOG_ERROR("Unexpected result from WaitForMultipleObjects: {}", waitForChangesOrQuit);
			}

			keepWaiting = false;
		}
	}
}

void TestEngineUpdate(Engine::EngineInstance& engineInstance)
{
	const float dt = 1.0f / 60.0f;
	engineInstance.Update(dt);
}

void HandleEditorInput(Editor::Context& context,
	std::unordered_map<std::string, Editor::EditorCommand>& executors_)
{
	while (!context.IsQuitRequested())
	{
		std::string command;

		std::cout << "Editor ready; command? ";
		std::getline(std::cin, command);
		const auto tokens = Split(command);

		TryExecute(tokens, executors_, context);
	}
}


int main()
{
	std::unordered_map<std::string, Editor::EditorCommand> executors_;
	std::unordered_map<std::string, Editor::CommandTaskFN> executors;

	executors_["echo"] = { Echo, "Log to console" };

	executors_["c"] = { Cls, "Clear screen" };
	executors_["cls"] = { Cls, "Clear screen" };
	executors_["clear"] = { Cls, "Clear screen" };

	executors_["q"] = { Quit, "Quit the editor" };
	executors_["quit"] = { Quit, "Quit the editor" };

	executors_["status"] = { Status, "Print status" };

	executors_["createProject"] = { CreateProject, "Create project" };
	executors_["cproj"] = { CreateProject, "Create project" };

	executors_["help"] = { Help, "Print list of commands" };
	executors_["h"] = { Help, "Print list of commands" };
	executors_["l"] = { Help, "Print list of commands" };
	executors_["list"] = { Help, "Print list of commands" };

	executors_["openProject"] = { OpenProject, "Open project" };
	executors_["oproj"] = { OpenProject, "Open project" };

	executors_["adb_pop"] = { PopulateAssetDatabase, "Populate asset database" };
	executors_["adb_list"] = { ListAssets, "List all assets in the asset database" };

	executors_["pname"] = { ProjectName, "Get / Set project name" };

	executors_["play"] = { TogglePlayMode, "Enter / exit play mode" };
	executors_["p"] = { PausePlayMode, "Pause / unpause play mode" };

	executors_["ls"] = { ListSceneNodes, "List scene hierarchy" };

	// TODO: Read this from an editor settings file; and make it part of an editorInstance
	Editor::Context context;

	Editor::ContextState contextState;
	contextState.gameTemplatePath = L"C:\\Users\\andra\\source\\repos\\Engine\\game-template";
	contextState.sdkPath = L"C:\\Dev\\EngineSdk\\sdk";

	//contextState.projectPath = L"C:\\Users\\andra\\source\\Foo";
	contextState.projectPath = L"";

	context.CollectExecutorInfo(executors_);

	const auto vsBundledCMakePath = GetVSBundledCmakePath();
	if (vsBundledCMakePath.has_value())
	{
		std::wcout << "Awesome!; cmake path: " << vsBundledCMakePath.value() << "\n";
		contextState.cmakePath = vsBundledCMakePath.value();
	}
	else
	{
		std::wcout << "Failed to get VS-bundled cmake path. Error: " << vsBundledCMakePath.error() << "\n";
	}

	context.SetContextState(contextState);

	Engine::EngineInstance::Initialize(std::make_unique<Engine::ComponentStorage>());

	// Start a thread to poll for directory changes
	std::thread directoryWatcherThread(PollDirectoryChanges, std::ref(context));

	// This is here until we move the console IO (std::cin) into another thread
	std::thread editorThread([&]()
		{
			while (!context.IsQuitRequested())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				auto chgEvents = context.ConsumeFileChangeEvents();
				Engine::EngineInstance::GetAssetDatabase().Update(chgEvents);
			}
		});

	std::thread inputThread(HandleEditorInput, std::ref(context), std::ref(executors_));

	// This is where we handle the editor state machine. Once it's done, let's
	// extract it into its own function.

	float dt = 0.0f;
	std::chrono::high_resolution_clock clock;
	auto lastTimeOnClock = clock.now();

	Engine::DataFile _sceneStateWhenEnteringPlayMode;
	Engine::DataFile _sceneStateBeforeHotReload;

	while (!context.IsQuitRequested())
	{
		const auto currentTime = clock.now();

		// TODO export into its own method
		if (context.HasProjectDllChanged())
		{
			LOG_INFO("Hot reload detected a change in the game DLL; reloading...");

			auto& scene = Engine::EngineInstance::GetActiveSceneRW();
			_sceneStateBeforeHotReload.Clear();
			Engine::SerializeScene(scene, _sceneStateBeforeHotReload);

			Engine::EngineInstance::Get().OnPlayModeStop();
			scene.Clear();										// Explicitly clearing the scene to get rid of 
																// dangling pointers to components that were in the old DLL.

			const auto reloadResult = context.ReloadProjectDll();
			if (!reloadResult.has_value())
			{
				LOG_ERROR("Failed to reload game DLL: {}", reloadResult.error().message);
			}
			else
			{
				auto sceneMaybe = Engine::DeserializeScene(_sceneStateBeforeHotReload, Engine::EngineInstance::GetComponentStorage());
				if (sceneMaybe.has_value())
				{
					Engine::EngineInstance::SetActiveScene(std::move(sceneMaybe.value()));
					LOG_INFO("Scene state restored after hot reload.");
				}
				else
				{
					LOG_ERROR("Failed to restore scene state after hot reload: {}", sceneMaybe.error().message);
				}
			}
		}


		Editor::EditorState editorState = context.GetEditorState();
		switch (editorState.currentState)
		{
		case Editor::EditorPlayState::Undefined:
		{
			LOG_INFO("Editor state is undefined; setting to Editing.");
			context.RequestEditorPlayModeStateChange(Editor::EditorPlayState::Editing);
			break;
		}

		case Editor::EditorPlayState::Editing:
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			break;
		}

		case Editor::EditorPlayState::StartingPlay:
		{
			LOG_INFO("Editor starting play mode...");
			// Serialize current scene state
			const auto& scene = Engine::EngineInstance::GetActiveScene();

			_sceneStateWhenEnteringPlayMode.Clear();
			Engine::SerializeScene(scene, _sceneStateWhenEnteringPlayMode);
			Engine::EngineInstance::Get().OnPlayModeStart();

			context.RequestEditorPlayModeStateChange(Editor::EditorPlayState::Playing);
			lastTimeOnClock = clock.now();

			break;
		}

		case Editor::EditorPlayState::Playing:
		{
			dt = std::chrono::duration<float>(currentTime - lastTimeOnClock).count();
			Engine::EngineInstance::Get().Update(dt);
			break;
		}

		case Editor::EditorPlayState::Paused:
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			break;
		}

		case Editor::EditorPlayState::StoppingPlay:
		{
			LOG_INFO("Editor exiting play mode...");

			Engine::EngineInstance::Get().OnPlayModeStop();
			Engine::EngineInstance::GetActiveSceneRW().Clear();

			// Deserialize and restore scene state from earlier.
			auto sceneMaybe = Engine::DeserializeScene(_sceneStateWhenEnteringPlayMode, Engine::EngineInstance::GetComponentStorage());
			if (sceneMaybe.has_value())
			{
				Engine::EngineInstance::SetActiveScene(std::move(sceneMaybe.value()));
			}
			else
			{
				LOG_ERROR("Failed to restore scene state after exiting play mode: {}", sceneMaybe.error().message);
			}

			context.RequestEditorPlayModeStateChange(Editor::EditorPlayState::Editing);
			break;
		}
		}

		context.ExecuteRequestedPlayModeStateChange();
		lastTimeOnClock = currentTime;
	}

	inputThread.join();
	editorThread.join();
	directoryWatcherThread.join();

	return 0;
}
