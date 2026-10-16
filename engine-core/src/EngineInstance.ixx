module;

#include "engine_core_api.h"

export module EngineInstance;

#if defined ( __INTELLISENSE__ )
#include "AssetDatabase.ixx"
#include "IComponentStorage.ixx"
#include "Scene.ixx"
#else
import AssetDatabase;
import IComponentStorage;
import Scene;
import std;
#endif

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

		void Run();
		void Update(float dt);
		void OnPlayModeStart();
		void OnPlayModeStop();

		bool IsQuitting() const;
		float CalculateDeltaTime() const;

		std::unique_ptr<IComponentStorage> _componentStorage;
		Scene::Scene* _activeScene{ nullptr };
		Engine::AssetDatabase _assetDatabase;

		double _elapsedSecondsSinceStarup{ 0.0 };
		double _elapsedSecondsInPlayMode{ 0.0 };
		double _elapsedSinceLastFixedUpdate{ 0.0 };
		double _fixedUpdateStep{ 1.0 / 60.0 };
	};

	export class ENGINE_CORE_API EngineInstance
	{
	public:
		static void Initialize(std::unique_ptr<IComponentStorage> componentStorage);
		static void Shutdown();
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

	private:
		EngineInstance() = default;
		EngineInstance(std::unique_ptr<IComponentStorage> componentStorage);
		~EngineInstance();

		static EngineInstance* _instance;

		EngineInstance_Impl* _impl{ nullptr };
	};

} // namespace Engine
