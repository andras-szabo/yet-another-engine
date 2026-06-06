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
	export struct GOImpl
	{
		GOImpl() = default;

		GOImpl(std::string_view name);
		std::string _name{ "GameObject" };
		std::vector<Component*> _components;		// Non-owning pointers!
	};

	GOImpl::GOImpl(std::string_view name)
		: _name{ name }
	{
	}

	export class ENGINE_CORE_API GameObject
	{
	public:
		GameObject();
		GameObject(std::string_view name, unsigned long long guid = 0);
		~GameObject();

		std::string_view GetName() const;
		GUID GetGUID() const;

		template<typename T, typename... Args>
			requires std::is_base_of_v<Component, T>
		T* AddComponent(IComponentStorage* componentStorage, Args&&...);

		template<typename T>
			requires std::is_base_of_v<Component, T>
		T* GetComponent();

		void SetTransform(Transform* trsf);
		Transform* GetTransform();
		std::vector<Component*> GetComponents();

	private:
		void SetGUID(GUID guid);

		GUID _guid;
		Transform* _transform{ nullptr };
		GOImpl* _gimpl{ nullptr };
	};

	template<typename T, typename... Args>
		requires std::is_base_of_v<Component, T>
	T* GameObject::AddComponent(IComponentStorage* componentStorage, Args&&... args)
	{
		assert(GetComponent<T>() == nullptr && "Adding multiple components of the same type to GameObjects is not supported.");
		assert(componentStorage != nullptr && "IComponentStorage is null pointer");

		Component* ptr = componentStorage->template CreateComponent<T>(std::forward<Args>(args)...);
		ptr->_owner = this;
		_gimpl->_components.push_back(ptr);
		ptr->OnCreate();

		return static_cast<T*>(ptr);
	}

	template<typename T>
		requires std::is_base_of_v<Component, T>
	T* GameObject::GetComponent()
	{
		for (auto* component : _gimpl->_components)
		{
			if (component->GetTypeID() == T::StaticTypeID())
			{
				return static_cast<T*>(component);
			}
		}

		return nullptr;
	}

} // namespace Engine

module :private;

namespace Engine
{
	GameObject::~GameObject()
	{
		delete _gimpl;
	}

	GameObject::GameObject() :
		_gimpl{ new GOImpl() }
	{
	}

	GameObject::GameObject(std::string_view name, unsigned long long guid)
		: _gimpl{ new GOImpl(name) }, _guid{ guid }
	{
	}

	void GameObject::SetGUID(GUID guid)
	{
		_guid = guid;
	}

	void GameObject::SetTransform(Transform* trsf)
	{
		_transform = trsf;
	}

	Engine::Transform* GameObject::GetTransform()
	{
		return _transform;
	}

	std::string_view GameObject::GetName() const
	{
		return _gimpl->_name;
	}

	GUID GameObject::GetGUID() const
	{
		return _guid;
	}

	std::vector<Component*> GameObject::GetComponents()
	{
		return _gimpl->_components;
	}
}
