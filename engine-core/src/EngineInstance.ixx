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

#pragma warning(push)
#pragma warning(disable: 4251)
		std::unique_ptr<IComponentStorage> _componentStorage;
#pragma warning(pop)

		Scene::Scene* _activeScene{ nullptr };
	};

} // namespace Engine
