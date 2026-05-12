module;

#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

#include "engine_core_api.h"

export module ComponentStorage;

import Component;
import IComponentStorage;

namespace Engine
{
	export class ENGINE_CORE_API ComponentStorage: public IComponentStorage
	{
	public:
		ComponentStorage() = default;
		ComponentStorage(const ComponentStorage& other) = delete;
		ComponentStorage& operator=(const ComponentStorage& other) = delete;

		void Foo() override {}

	protected:
		Component* CreateComponentImpl(std::function<std::unique_ptr<Component>()> factory) override;
#pragma warning(push)
#pragma warning(disable: 4251)
		std::vector<std::unique_ptr<Component>> _components;
#pragma warning(pop)
	};

	Component* ComponentStorage::CreateComponentImpl(std::function<std::unique_ptr<Component>()> factory)
	{	
		auto& newComponent = _components.emplace_back(factory());
		return newComponent.get();
	}
} // namespace Engine