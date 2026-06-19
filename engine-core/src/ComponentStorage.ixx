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
	struct ComponentStorage_Impl
	{
		std::vector<std::unique_ptr<Component>> components;
	};

	export class ENGINE_CORE_API ComponentStorage: public IComponentStorage
	{
	public:
		ComponentStorage();
		~ComponentStorage();

		ComponentStorage(const ComponentStorage& other) = delete;
		ComponentStorage& operator=(const ComponentStorage& other) = delete;

		void DestroyComponent(Component* component) override;

	protected:
		Component* CreateComponentImpl(std::function<std::unique_ptr<Component>()> factory) override;
		ComponentStorage_Impl* _components{ nullptr };
	};

} // namespace Engine