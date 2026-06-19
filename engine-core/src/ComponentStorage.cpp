module;

module ComponentStorage;

#if defined ( __INTELLISENSE__ )
#include "Component.ixx"
#include "IComponentStorage.ixx"
#include "ComponentStorage.ixx"
#else
import Component;
import IComponentStorage;

import std;
#endif

namespace Engine
{
	ComponentStorage::ComponentStorage()
	{
		_components = new ComponentStorage_Impl;
		_components->components.reserve(1024);
	}

	ComponentStorage::~ComponentStorage()
	{
		delete _components;
		_components = nullptr;
	}

	Component* ComponentStorage::CreateComponentImpl(std::function<std::unique_ptr<Component>()> factory)
	{
		auto& newComponent = _components->components.emplace_back(factory());
		Component* rawPtr = newComponent.get();
		return rawPtr;
	}

	void ComponentStorage::DestroyComponent(Component* component)
	{
		component->OnDestroy();
		std::erase_if(_components->components, 
			[component](const auto& ptr) 
			{ 
				return ptr.get() == component; 
			});
	}
} // namespace Engine