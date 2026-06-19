#include <cassert>
#include "engine_core_api.h"

module EngineInstance;

#if defined ( __INTELLISENSE__ )
#include "IComponentStorage.ixx"
#include "Scene.ixx"
#else
import IComponentStorage;
import Scene;
#endif

import std;

namespace Engine
{
	EngineInstance::~EngineInstance()
	{
		delete _activeScene;
	}

	IComponentStorage& EngineInstance::GetComponentStorage()
	{
		assert(_componentStorage != nullptr && "Component storage null; engine instance not initialized?");
		return *_componentStorage;
	}

	Scene::Scene& EngineInstance::GetActiveScene()
	{
		assert(_activeScene != nullptr && "Active scene is null; engine instance not initialized?");
		return *_activeScene;
	}

	void EngineInstance::Initialize(std::unique_ptr<IComponentStorage> componentStorage)
	{
		_componentStorage = std::move(componentStorage);

		delete _activeScene;
		_activeScene = nullptr;

		_activeScene = new Engine::Scene::Scene(_componentStorage.get(), "UntitledScene");
	}

	void EngineInstance::Shutdown()
	{
		//TODO
	}
} // namespace Engine