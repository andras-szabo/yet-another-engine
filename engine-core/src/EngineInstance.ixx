module;

#include <cassert>
#include <memory>

#include "engine_core_api.h"

export module EngineInstance;

import IComponentStorage;
import IScene;

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
		void Initialize(IScene* activeScene,
			std::unique_ptr<IComponentStorage> componentStorage);

		void Shutdown();

		IComponentStorage& GetComponentStorage();
		IScene& GetActiveScene();

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::unique_ptr<IComponentStorage> _componentStorage;
#pragma warning(pop)

		IScene* _activeScene;
	};

	export ENGINE_CORE_API EngineInstance Instance;

	IComponentStorage& EngineInstance::GetComponentStorage()
	{
		assert(_componentStorage != nullptr && "Component storage null; engine instance not initialized?");
		return *_componentStorage;
	}

	IScene& EngineInstance::GetActiveScene()
	{
		return *_activeScene;
	}

	void EngineInstance::Initialize(IScene* activeScene,
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
