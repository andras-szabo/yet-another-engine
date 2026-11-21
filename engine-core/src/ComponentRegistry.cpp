module;

#include <cassert>

#include "LoggerMacros.h"
#include "ReflectionMacros.h"

module ComponentRegistry;

#if defined ( __INTELLISENSE__ )
#include "EngineError.ixx"
#include "Logger.ixx"
#include "IComponentStorage.ixx"
#include "ComponentRegistry.ixx"

#include <algorithm>
#else
import Component;
import Error;
import IComponentStorage;
import Logger;

import std;
#endif

namespace Engine
{
	Expected<std::vector<ComponentInfo>> ComponentRegistry::GetRegisteredComponents(bool sortedByName) const
	{
		std::vector<ComponentInfo> components;

		for (const auto& [typeID, name] : _componentNames)
		{
			components.push_back({ typeID, name });
		}

		if (sortedByName)
		{
			std::sort(components.begin(), components.end(), [](const ComponentInfo& a, const ComponentInfo& b) {
				return a.name < b.name;
			});
		}

		return components;
	}

	Expected<Component*> ComponentRegistry::Create(unsigned int typeID, IComponentStorage& storage)
	{
		const auto item = _factories.find(typeID);
		if (item == _factories.end())
		{
			return Unexpected({ ErrorType::NotFound, std::format("No component found with type ID: {}", typeID) });
		}

		auto* component = storage.CreateComponentDynamic((*item).second);
		if (component == nullptr)
		{
			return Unexpected({ ErrorType::Undefined, std::format("Error when creating component with type ID: {}", typeID) });
		}

		return component;
	}

	void ComponentRegistry::Register(std::string_view name, unsigned int typeID, ComponentFactoryFn fn)
	{
		// TODO - optimize away std::format allocation
		const auto existingTypeName = _componentNames.find(typeID);
		if (existingTypeName != _componentNames.end())
		{
			assert(false && std::format("Double component registration! {} and {} share ID: {}", existingTypeName->second, name, typeID).c_str());
		}

		assert(_factories.find(typeID) == _factories.end());

		_factories.insert({ typeID, fn });
		_componentNames.insert({ typeID, std::string(name) });

		LOG_TRACE("Registering component type {} ({})", name, typeID);
	}

	void ComponentRegistry::Unregister(unsigned int typeID)
	{
		auto item = _factories.find(typeID);
		if (item != _factories.end())
		{
			_factories.erase(item);
			_componentNames.erase(typeID);

			LOG_TRACE("Unregistering component type ID {}", typeID);
		}
	}

	bool ComponentRegistry::Has(unsigned int typeID) const
	{
		return _factories.find(typeID) != _factories.end();
	}

	ComponentRegistry& GlobalComponentRegistry()
	{
		static ComponentRegistry globalComponentRegistry;
		return globalComponentRegistry;
	}

	void RegisterComponent(std::string_view name, unsigned int typeID, ComponentFactoryFn factory)
	{
		GlobalComponentRegistry().Register(name, typeID, factory);
	}

	void UnregisterComponent(unsigned int typeID)
	{
		GlobalComponentRegistry().Unregister(typeID);
	}

} // namespace Engine