module;

#include <cassert>
#include <concepts>
#include <string>
#include <type_traits>
#include <vector>

#include "engine_core_api.h"

export module GameObject;

import Component;
import IComponentStorage;
import GUID;
import Transform;

namespace Engine
{
	export class ENGINE_CORE_API GameObject
	{
	public:
		GameObject();
		GameObject(std::string_view name);

		std::string_view GetName() const;
		GUID GetGUID() const;
		
		template<typename T, typename... Args>
		requires std::is_base_of_v<Component, T>
		T* AddComponent(IComponentStorage* componentStorage, Args&&...);

		template<typename T>
		requires std::is_base_of_v<Component, T>
		T* GetComponent();

		Transform* GetTransform();

	private:
		GUID _guid;
		Transform* _transform{ nullptr };

#pragma warning(push)
#pragma warning(disable: 4251)		// 4251: complaining about no DLL access to private members
		std::string _name{ "GameObject" };
		std::vector<Component*> _components;		// Non-owning pointers!
#pragma warning(pop)
	};

	GameObject::GameObject():
		_name{ "-root-" }
	{
	}

	GameObject::GameObject(std::string_view name)		
		: _name{ name }
	{
	}

	Engine::Transform* GameObject::GetTransform()
	{
		return _transform;
	}

	template<typename T, typename... Args>
	requires std::is_base_of_v<Component, T>
	T* GameObject::AddComponent(IComponentStorage* componentStorage, Args&&... args)
	{
		assert(GetComponent<T>() == nullptr && "Adding multiple components of the same type to GameObjects is not supported.");
		assert(componentStorage != nullptr && "IComponentStorage is null pointer");

		Component* ptr = componentStorage->template CreateComponent<T>(std::forward<Args>(args)...);
		ptr->_owner = this;
		_components.push_back(ptr);
		ptr->OnCreate();

		return static_cast<T*>(ptr);
	}

	template<typename T>
	requires std::is_base_of_v<Component, T>
	T* GameObject::GetComponent()
	{
		for (auto* component : _components)
		{
			if (component->GetTypeID() == T::StaticTypeID())
			{
				return static_cast<T*>(component);
			}
		}

		return nullptr;
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
