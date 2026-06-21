module;

#include <cassert>
#include "engine_core_api.h"

export module GameObject;

#if defined ( __INTELLISENSE__ )
#include <concepts>
#include <string>
#include <type_traits>
#include <vector>

#include "Component.ixx"
#include "IComponentStorage.ixx"
#include "GUID.ixx"
#include "Transform.ixx"
#else
import Component;
import IComponentStorage;
import GUID;
import Transform;

import std;
#endif

namespace Engine
{
	export struct GOImpl
	{
		GOImpl() = default;

		GOImpl(std::string_view name);
		std::string _name{ "GameObject" };
		std::vector<Component*> _components;		// Non-owning pointers!
	};

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

		void AddComponentRaw(Component* component);
		void SetTransform(Transform* trsf);
		Transform* GetTransform();
		std::vector<Component*> GetComponents();
		const std::vector<Component*> GetComponents() const;

	private:
		void SetGUID(GUID guid);

		GUID _guid;
		Transform* _transform{ nullptr };
		GOImpl* _gimpl{ nullptr };
		IComponentStorage* _storage{ nullptr };
	};

	template<typename T, typename... Args>
		requires std::is_base_of_v<Component, T>
	T* GameObject::AddComponent(IComponentStorage* componentStorage, Args&&... args)
	{
		assert(GetComponent<T>() == nullptr && "Adding multiple components of the same type to GameObjects is not supported.");
		assert(componentStorage != nullptr && "IComponentStorage is null pointer");

		if (_storage == nullptr) _storage = componentStorage;

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

