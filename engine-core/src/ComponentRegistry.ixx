module;

#include <cassert>
#include <format>
#include <functional>
#include <memory>
#include <unordered_map>

#include "engine_core_api.h"
#include "LoggerMacros.h"
#include "ReflectionMacros.h"

export module ComponentRegistry;

import Component;
import Error;
import Logger;
import IComponentStorage;

namespace Engine
{
	export using ComponentFactoryFn = std::function<std::unique_ptr<Component>()>;

	export class ENGINE_CORE_API ComponentRegistry
	{
	public:
		void Register(unsigned int typeID, ComponentFactoryFn fn);
		void Unregister(unsigned int typeID);

		Expected<Component*> Create(unsigned int typeID, IComponentStorage& storage);
		bool Has(unsigned int typeID) const;

	private:
		std::unordered_map<unsigned int, ComponentFactoryFn> _factories;
	};

	export ENGINE_CORE_API ComponentRegistry GlobalComponentRegistry;

	export ENGINE_CORE_API 
	void RegisterComponent(unsigned int typeID, ComponentFactoryFn factory)
	{
		GlobalComponentRegistry.Register(typeID, factory);
	}

	export ENGINE_CORE_API void UnregisterComponent(unsigned int typeID)
	{
		GlobalComponentRegistry.Unregister(typeID);
	}
} // namespace Engine

module :private;

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
}// namespace Engine