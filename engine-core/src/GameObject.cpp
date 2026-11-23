module;

#include "LoggerMacros.h"

module GameObject;

#if defined ( __INTELLISENSE__ )
#include <string>
#include "ComponentRegistry.ixx"
#include "GameObject.ixx"
#include "Logger.ixx"
#else
import Component;
import ComponentRegistry;
import Logger;
import GUID;
import IComponentStorage;
import Transform;

import std;
#endif

namespace Engine
{
	GOImpl::GOImpl(std::string_view name)
	{
	}

	GameObject::~GameObject()
	{
		if (_gimpl != nullptr && _storage != nullptr)
		{
			for (Component* c : _gimpl->_components)
			{
				_storage->DestroyComponent(c);
			}
		}
		delete _gimpl;
	}

	GameObject::GameObject() :
		_gimpl{ new GOImpl() }
	{
	}

	GameObject::GameObject(std::string_view name, unsigned long long _guid)
		: _gimpl{ new GOImpl(name) }, _guid{ _guid }
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
		return _transform->GetName();
	}

	GUID GameObject::GetGUID() const
	{
		return _guid;
	}

	const std::vector<Component*> GameObject::GetComponents() const
	{
		return _gimpl->_components;
	}

	std::vector<Component*> GameObject::GetComponents()
	{
		return _gimpl->_components;
	}

	const Component* GameObject::GetComponent(unsigned int typeID) const
	{
		for (const auto* component : _gimpl->_components)
		{
			if (component->GetTypeID() == typeID)
			{
				return component;
			}
		}

		return nullptr;
	}

	Component* GameObject::GetComponentRW(unsigned int typeID)
	{
		const auto* component = GetComponent(typeID);

		if (component != nullptr)
		{
			return const_cast<Component*>(component);
		}

		return nullptr;
	}

	void GameObject::AddComponentRaw(Component* component)
	{
		component->_owner = this;
		_gimpl->_components.push_back(component);

		// Do not call component->OnCreate here. The idea is that you can use AddComponentRaw
		// to create a placeholder of a component, into which we can then deserialize actual
		// component data, and call OnCreate when that is done. (Serialization.ixx, 
		// DeserializeScene.)
	}

	bool GameObject::TryRemoveComponent(unsigned int typeID)
	{
		if (_storage == nullptr)
		{
			LOG_ERROR("IComponentStorage is null pointer. Cannot remove compnent.");
			return false;
		}

		if (typeID == Transform::StaticTypeID())
		{
			LOG_ERROR("Cannot remove Transform component from GameObject.");
			return false;
		}

		for (auto it = _gimpl->_components.begin(); it != _gimpl->_components.end(); ++it)
		{
			if ((*it)->GetTypeID() == typeID)
			{
				_storage->DestroyComponent(*it);
				_gimpl->_components.erase(it);
				return true;
			}
		}
		return false;
	}

	bool GameObject::HasComponent(unsigned int typeID) const
	{
		for (const auto* component : _gimpl->_components)
		{
			if (component->GetTypeID() == typeID)
			{
				return true;
			}
		}

		return false;
	}

	bool GameObject::TryAddComponent(unsigned int typeID)
	{
		if (_storage == nullptr)
		{
			LOG_ERROR("IComponentStorage is null pointer. Cannot add component.");
			return false;
		}

		if (HasComponent(typeID))
		{
			LOG_ERROR("GameObject already has a component of type ID {}. Cannot add another.", typeID);
			return false;
		}

		const auto result = GlobalComponentRegistry().Create(typeID, *_storage);
		if (!result.has_value())
		{
			LOG_ERROR("Failed to create component with type ID {}: {}", typeID, result.error().message);
			return false;
		}

		auto* component = result.value();

		component->_owner = this;
		_gimpl->_components.push_back(component);
		component->OnCreate();

		return true;
	}
}