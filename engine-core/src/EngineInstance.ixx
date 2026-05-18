module;

#include <cassert>
#include <memory>

#include "engine_core_api.h"

export module EngineInstance;

import IComponentStorage;
import Scene;

namespace Engine
{
	export class ENGINE_CORE_API EngineInstance
	{
	public:
		EngineInstance() = default;
		~EngineInstance() = default;
		EngineInstance(const EngineInstance& other) = delete;
		EngineInstance& operator=(const EngineInstance& other) = delete;

		//TODO - scene ownership
		void Initialize(Scene::Scene* activeScene,
			std::unique_ptr<IComponentStorage> componentStorage);

		void Shutdown();

		IComponentStorage& GetComponentStorage();
		Scene::Scene& GetActiveScene();

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::unique_ptr<IComponentStorage> _componentStorage;
#pragma warning(pop)

		Scene::Scene* _activeScene{ nullptr };
	};

	export ENGINE_CORE_API EngineInstance Instance;

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

	void EngineInstance::Initialize(Scene::Scene* activeScene,
		std::unique_ptr<IComponentStorage> componentStorage)
	{
		_activeScene = activeScene;
		_componentStorage = std::move(componentStorage);
	}

	void EngineInstance::Shutdown()
	{
		//TODO
	}
} // namespace Engine
