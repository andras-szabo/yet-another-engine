module;

#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

#include "engine_core_api.h"

export module ComponentStorage;

#if defined ( __INTELLISENSE__ )
#include "Component.ixx"
#include "IComponentStorage.ixx"
#else
import Component;
import IComponentStorage;
#endif

namespace Engine
{
	export class ENGINE_CORE_API ComponentStorage: public IComponentStorage
	{
	public:
		ComponentStorage() = default;
		ComponentStorage(const ComponentStorage& other) = delete;
		ComponentStorage& operator=(const ComponentStorage& other) = delete;

		void DestroyComponent(Component* component) override;

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
		Component* rawPtr = newComponent.get();
		return rawPtr;
	}

	void ComponentStorage::DestroyComponent(Component* component)
	{
		component->OnDestroy();
		std::erase_if(_components, [component](const auto& ptr) { return ptr.get() == component; });
	}
} // namespace Engine