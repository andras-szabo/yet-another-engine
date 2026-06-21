module;

module IComponentStorage;

#if defined ( __INTELLISENSE__ )
#include "Component.ixx"
#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include "IComponentStorage.ixx"
#else
import Component;
import std;
#endif

namespace Engine
{
	Component* IComponentStorage::CreateComponentDynamic(ComponentFactoryFn factory)
	{
		return CreateComponentImpl(factory);
	}
} // namespace Engine