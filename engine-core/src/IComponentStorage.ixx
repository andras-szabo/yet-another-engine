module;

#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>

#include "engine_core_api.h"

export module IComponentStorage;

import Component;

namespace Engine
{
	export class ENGINE_CORE_API IComponentStorage
	{
	public:
		virtual ~IComponentStorage() = default;
		
		template <typename T, typename... Args>
		requires std::is_base_of_v<Component, T>
		Component* CreateComponent(Args&&... args);

		virtual void Foo() = 0;

	protected:
		virtual Component* CreateComponentImpl(std::function<std::unique_ptr<Component>()> factory) = 0;
	};

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