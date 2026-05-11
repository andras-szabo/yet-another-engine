module;

#include <concepts>
#include <string>
#include <type_traits>
#include <vector>

#include "engine_core_api.h"

export module GameObject;

import Component;
import ComponentStorage;
import EngineInstance;
import GUID;

namespace Engine
{
	export class ENGINE_CORE_API GameObject
	{
	public:
		std::string_view GetName() const;
		GUID GetGUID() const;
		
		template<typename T, typename... Args>
		requires std::is_base_of_v<Component, T>
		T* AddComponent(Args&&...);

	private:
		GUID _guid;
		std::string _name;
		std::vector<Component*> _components;		// Non-owning pointers!

	};

	template<typename T, typename... Args>
	requires std::is_base_of_v<Component, T>
	T* GameObject::AddComponent(Args&&... args)
	{
		Component* ptr = Instance.GetComponentStorage().CreateComponent<T>(std::forward<Args>(args)...);
		_components.push_back(ptr);
		return static_cast<T*>(ptr);
	}

	std::string_view GameObject::GetName() const
	{
		return _name;
	}

	GUID GameObject::GetGUID() const
	{
		return _guid;
	}
} // namespace Engine
