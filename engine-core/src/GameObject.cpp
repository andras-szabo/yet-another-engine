module;

module GameObject;

#if defined ( __INTELLISENSE__ )
#include <string>
#include "GameObject.ixx"
#else
import Component;
import IComponentStorage;
import GUID;
import Transform;

import std;
#endif

namespace Engine
{
	GOImpl::GOImpl(std::string_view name)
	: _name{ name }
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

	const std::vector<Component*> GameObject::GetComponents() const
	{
		return _gimpl->_components;
	}

	std::vector<Component*> GameObject::GetComponents()
	{
		return _gimpl->_components;
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
}