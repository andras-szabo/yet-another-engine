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

	export struct ComponentInfo
	{
		unsigned int typeID;
		std::string name;
	};

	export class ComponentRegistry
	{
	public:
		ENGINE_CORE_API ComponentRegistry() = default;
		ENGINE_CORE_API ~ComponentRegistry() = default;

		ComponentRegistry(const ComponentRegistry& other) = delete;
		ComponentRegistry& operator=(const ComponentRegistry& other) = delete;

		ENGINE_CORE_API ComponentRegistry(ComponentRegistry&& other) = default;
		ENGINE_CORE_API ComponentRegistry& operator=(ComponentRegistry&& other) = default;

		ENGINE_CORE_API void Register(std::string_view name, unsigned int typeID, ComponentFactoryFn fn);
		ENGINE_CORE_API void Unregister(unsigned int typeID);

		ENGINE_CORE_API Expected<Component*> Create(unsigned int typeID, IComponentStorage& storage);
		ENGINE_CORE_API bool Has(unsigned int typeID) const;
		ENGINE_CORE_API Expected<std::vector<ComponentInfo>> GetRegisteredComponents(bool sortedByName) const;

	private:
		std::unordered_map<unsigned int, ComponentFactoryFn> _factories;
		std::unordered_map<unsigned int, std::string> _componentNames;
	};

	export ENGINE_CORE_API ComponentRegistry& GlobalComponentRegistry();

	export ENGINE_CORE_API 
	void RegisterComponent(std::string_view name, unsigned int typeID, ComponentFactoryFn factory);

	export ENGINE_CORE_API void UnregisterComponent(unsigned int typeID);

}// namespace Engine