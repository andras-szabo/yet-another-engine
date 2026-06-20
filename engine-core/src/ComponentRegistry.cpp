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
#else
import Component;
import Error;
import IComponentStorage;
import Logger;

import std;
#endif

namespace Engine
{
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

	void ComponentRegistry::Register(unsigned int typeID, ComponentFactoryFn fn)
	{
		// TODO - optimize away std::format allocation
		assert(_factories.find(typeID) == _factories.end() && std::format("Double component registration! Type ID: {}", typeID).c_str());

		_factories.insert({ typeID, fn });

		LOG_TRACE("Registering type ID {}", typeID);
	}

	void ComponentRegistry::Unregister(unsigned int typeID)
	{
		auto item = _factories.find(typeID);
		if (item != _factories.end())
		{
			 _factories.erase(item);
			LOG_TRACE("Unregistering type ID {}", typeID);
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

	void RegisterComponent(unsigned int typeID, ComponentFactoryFn factory)
	{
		GlobalComponentRegistry().Register(typeID, factory);
	}

	void UnregisterComponent(unsigned int typeID)
	{
		GlobalComponentRegistry().Unregister(typeID);
	}

} // namespace Engine