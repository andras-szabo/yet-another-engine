#include <cassert>
#include "engine_core_api.h"

module EngineInstance;

#if defined ( __INTELLISENSE__ )
#include "IComponentStorage.ixx"
#include "EngineInstance.ixx"
#include "Scene.ixx"
#else
import IComponentStorage;
import Scene;
#endif

import std;

namespace Engine
{
	// Nullptr is a compile-time constant, so this counts as constant
	// initialization that should happen when the .dll is loaded, avoiding
	// the SOIF.
	EngineInstance* EngineInstance::_instance = nullptr;

	void EngineInstance::Initialize(std::unique_ptr<IComponentStorage> componentStorage)
	{
		if (_instance == nullptr)
		{
			_instance = new EngineInstance(std::move(componentStorage));
		}
	}

	void EngineInstance::Shutdown()
	{
		delete _instance;
		_instance = nullptr;
	}

	EngineInstance& EngineInstance::Get()
	{
		return *_instance;
	}

	EngineInstance::~EngineInstance()
	{
		delete _activeScene;
		_activeScene = nullptr;
	}

	IComponentStorage& EngineInstance::GetComponentStorage()
	{
		assert(_instance->_componentStorage != nullptr && "Component storage null; engine instance not initialized?");
		return *(_instance->_componentStorage);
	}

	const Scene::Scene& EngineInstance::GetActiveScene()
	{
		assert(_instance->_activeScene != nullptr && "Active scene is null; engine instance not initialized?");
		return *(_instance->_activeScene);
	}

	Scene::Scene& EngineInstance::GetActiveSceneRW()
	{
		assert(_instance->_activeScene != nullptr && "Active scene is null; engine instance not initialized?");
		return *(_instance->_activeScene);
	}

	void EngineInstance::SetActiveScene(Scene::Scene&& scene)
	{
		assert(_instance != nullptr && "Engine instance is not initialized!");
		delete _instance->_activeScene;
		_instance->_activeScene = new Engine::Scene::Scene(std::forward<Scene::Scene>(scene));

	}

	EngineInstance::EngineInstance(std::unique_ptr<IComponentStorage> componentStorage)
	{
		_componentStorage = std::move(componentStorage);
		_activeScene = new Engine::Scene::Scene(_componentStorage.get(), "UntitledScene");
	}

} // namespace Engine