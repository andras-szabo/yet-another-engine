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
		EngineInstance() = default;
		~EngineInstance();
		EngineInstance(const EngineInstance& other) = delete;
		EngineInstance& operator=(const EngineInstance& other) = delete;

		// TODO scene ownership?
		void Initialize(std::unique_ptr<IComponentStorage> componentStorage);

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


} // namespace Engine
