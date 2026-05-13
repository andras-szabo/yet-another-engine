module;

#include <cassert>
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
import Transform;

namespace Engine
{
	export class ENGINE_CORE_API GameObject
	{
	public:
		GameObject();

		std::string_view GetName() const;
		GUID GetGUID() const;
		
		template<typename T, typename... Args>
		requires std::is_base_of_v<Component, T>
		T* AddComponent(Args&&...);

		template<typename T>
		requires std::is_base_of_v<Component, T>
		T* GetComponent();

		Transform* GetTransform();

	private:
		GUID _guid;
		Transform* _transform{ nullptr };

#pragma warning(push)
#pragma warning(disable: 4251)		// 4251: complaining about no DLL access to private members
		std::string _name;
		std::vector<Component*> _components;		// Non-owning pointers!
#pragma warning(pop)

	};

	GameObject::GameObject()
	{
		_transform = AddComponent<Engine::Transform>();
	}

	Engine::Transform* GameObject::GetTransform()
	{
		return _transform;
	}

	template<typename T, typename... Args>
	requires std::is_base_of_v<Component, T>
	T* GameObject::AddComponent(Args&&... args)
	{
		assert(GetComponent<T>() == nullptr && "Adding multiple components of the same type to GameObjects is not supported.");

		Component* ptr = Instance.GetComponentStorage().CreateComponent<T>(std::forward<Args>(args)...);
		_components.push_back(ptr);
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
