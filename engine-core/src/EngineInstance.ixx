module;

#include <memory>

#include "engine_core_api.h"

export module EngineInstance;

import IComponentStorage;

namespace Engine
{
	export class ENGINE_CORE_API EngineInstance
	{
	public:
		EngineInstance() = default;
		EngineInstance(const EngineInstance& other) = delete;
		EngineInstance& operator=(const EngineInstance& other) = delete;

		void Initialize(std::unique_ptr<IComponentStorage> componentStorage);
		void Shutdown();

		IComponentStorage& GetComponentStorage();

	private:
		std::unique_ptr<IComponentStorage> _componentStorage;
	};

	export ENGINE_CORE_API EngineInstance Instance;

	IComponentStorage& EngineInstance::GetComponentStorage()
	{
		return *_componentStorage;
	}
} // namespace Engine
