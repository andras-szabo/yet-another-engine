module;

#include "engine_core_api.h"

export module EngineInstance;

#if defined ( __INTELLISENSE__ )
#include "AssetDatabase.ixx"
#include "IComponentStorage.ixx"
#include "EngineError.ixx"
#include "Scene.ixx"
#else
import AssetDatabase;
import ComponentStorage;
import IComponentStorage;
import Error;
import Logger;
import Scene;

import std;
#endif

namespace Engine
{
	struct EngineInstance_Impl;

	export class ENGINE_CORE_API EngineInstance
	{
	public:
		static void Initialize();
		static void Initialize(std::unique_ptr<IComponentStorage> componentStorage);
		static Engine::Expected<void> Shutdown();

		static EngineInstance& Get();
		static IComponentStorage& GetComponentStorage();
		static AssetDatabase& GetAssetDatabase();
		static void SetActiveScene(Scene::Scene&& scene);
		static const Scene::Scene& GetActiveScene();
		static Scene::Scene& GetActiveSceneRW();

		EngineInstance(const EngineInstance& other) = delete;
		EngineInstance& operator=(const EngineInstance& other) = delete;
		
		void Run();
		void Update(float dt);

		void OnPlayModeStart();
		void OnPlayModeStop();

		void OnApplicationStart();
		void OnApplicationQuit();

	private:
		EngineInstance() = default;
		EngineInstance(std::unique_ptr<IComponentStorage> componentStorage);
		~EngineInstance();

		static EngineInstance* _instance;

		EngineInstance_Impl* _impl{ nullptr };
	};

} // namespace Engine
