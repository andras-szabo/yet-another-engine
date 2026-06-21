module;


#include "engine_core_api.h"

export module IComponentStorage;

#if defined ( __INTELLISENSE__ )
#include "Component.ixx"
#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#else
import Component;
import std;
#endif

namespace Engine
{
	using ComponentFactoryFn = std::function<std::unique_ptr<Component>()>;

	export class ENGINE_CORE_API IComponentStorage
	{
	public:
		virtual ~IComponentStorage() = default;
		
		template <typename T, typename... Args>
		requires std::is_base_of_v<Component, T>
		Component* CreateComponent(Args&&... args);
		Component* CreateComponentDynamic(ComponentFactoryFn factory);

		// Calls OnDestroy() on the component, then removes it from storage.
		// Must be called before the owning GameObject is destroyed.
		virtual void DestroyComponent(Component* component) = 0;

	protected:
		virtual Component* CreateComponentImpl(std::function<std::unique_ptr<Component>()> factory) = 0;
	};

	Component* IComponentStorage::CreateComponentDynamic(ComponentFactoryFn factory)
	{
		return CreateComponentImpl(factory);
	}

	template<typename T, typename... Args>
	requires std::is_base_of_v<Component, T>
	Component* IComponentStorage::CreateComponent(Args&&... args)
	{
		auto factory = [&]() -> std::unique_ptr<T>
		{
			return std::make_unique<T>(std::forward<Args>(args)...);
		};

		Component* rawPtr = CreateComponentImpl(factory);
		return static_cast<T*>(rawPtr);
	}
} // namespace Engine