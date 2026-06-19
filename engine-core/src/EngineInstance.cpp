module;

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

	// static
	void EngineInstance::Initialize(std::unique_ptr<IComponentStorage> componentStorage)
	{
		if (_instance == nullptr)
		{
			_instance = new EngineInstance(std::move(componentStorage));
		}
	}

	// static
	void EngineInstance::Shutdown()
	{
		delete _instance;
		_instance = nullptr;
	}

	// static
	EngineInstance& EngineInstance::Get()
	{
		return *_instance;
	}

	// static
	IComponentStorage& EngineInstance::GetComponentStorage()
	{
		assert(_instance != nullptr && "Engine instance not initialized?");
		return *(_instance->_impl->_componentStorage.get());
	}

	// static
	const Scene::Scene& EngineInstance::GetActiveScene()
	{
		assert(_instance != nullptr && "Engine instance not initialized?");
		return *(_instance->_impl->_activeScene);
	}

	// static
	Scene::Scene& EngineInstance::GetActiveSceneRW()
	{
		assert(_instance != nullptr && "Engine instance not initialized?");
		return *(_instance->_impl->_activeScene);
	}

	// static
	void EngineInstance::SetActiveScene(Scene::Scene&& scene)
	{
		assert(_instance != nullptr && "Engine instance is not initialized!");
		_instance->_impl->SetActiveScene(std::forward<Scene::Scene&&>(scene));
	}

	EngineInstance::~EngineInstance()
	{
		delete _impl;
		_impl = nullptr;
	}

	EngineInstance::EngineInstance(std::unique_ptr<IComponentStorage> componentStorage)
	{
		_impl = new EngineInstance_Impl(std::forward<std::unique_ptr<IComponentStorage>>(componentStorage));
	}

} // namespace Engine