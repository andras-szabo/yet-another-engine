module;

#include <cassert>
#include <format>
#include <functional>
#include <memory>
#include <unordered_map>

#include "engine_core_api.h"

export module ComponentRegistry;

#if defined ( __INTELLISENSE__ )
#include "Component.ixx"
#include "EngineError.ixx"
#include "IComponentStorage.ixx"
#else
import Component;
import Error;
import IComponentStorage;

import std;
#endif

namespace Engine
{
	export using ComponentFactoryFn = std::function<std::unique_ptr<Component>()>;

	struct ComponentRegistry_Impl
	{
		std::unordered_map<unsigned int, ComponentFactoryFn> _factories;
	};

	export class ENGINE_CORE_API ComponentRegistry
	{
	public:
		ComponentRegistry();
		~ComponentRegistry();

		ComponentRegistry(const ComponentRegistry& other) = delete;
		ComponentRegistry& operator=(const ComponentRegistry& other) = delete;

		ComponentRegistry(ComponentRegistry&& other);
		ComponentRegistry& operator=(ComponentRegistry&& other);

		void Register(unsigned int typeID, ComponentFactoryFn fn);
		void Unregister(unsigned int typeID);

		Expected<Component*> Create(unsigned int typeID, IComponentStorage& storage);
		bool Has(unsigned int typeID) const;

	private:
		ComponentRegistry_Impl* _registry{ nullptr };
	};

	export ENGINE_CORE_API ComponentRegistry& GlobalComponentRegistry();

	export ENGINE_CORE_API 
	void RegisterComponent(unsigned int typeID, ComponentFactoryFn factory);

	export ENGINE_CORE_API void UnregisterComponent(unsigned int typeID);

}// namespace Engine