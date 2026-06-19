module;

#include "engine_core_api.h"

export module EngineInstance;

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
	struct EngineInstance_Impl
	{
		EngineInstance_Impl(std::unique_ptr<IComponentStorage> componentStorage)
			: _componentStorage(std::move(componentStorage))
		{
			_activeScene = new Engine::Scene::Scene(_componentStorage.get(), "UntitledScene");
		}

		~EngineInstance_Impl()
		{
			delete _activeScene;
			_activeScene = nullptr;
		}

		void SetActiveScene(Scene::Scene&& scene)
		{
			delete _activeScene;
			_activeScene = new Engine::Scene::Scene(std::forward<Scene::Scene&&>(scene));
		}

		std::unique_ptr<IComponentStorage> _componentStorage;
		Scene::Scene* _activeScene{ nullptr };
	};

	export class ENGINE_CORE_API EngineInstance
	{
	public:
		static void Initialize(std::unique_ptr<IComponentStorage> componentStorage);
		static void Shutdown();
		static EngineInstance& Get();
		static IComponentStorage& GetComponentStorage();
		static void SetActiveScene(Scene::Scene&& scene);
		static const Scene::Scene& GetActiveScene();
		static Scene::Scene& GetActiveSceneRW();

		EngineInstance(const EngineInstance& other) = delete;
		EngineInstance& operator=(const EngineInstance& other) = delete;

	private:
		EngineInstance() = default;
		EngineInstance(std::unique_ptr<IComponentStorage> componentStorage);
		~EngineInstance();

		static EngineInstance* _instance;

		EngineInstance_Impl* _impl{ nullptr };
	};

} // namespace Engine
