module;

#include <span>

#include "engine_core_api.h"

export module Component;

import Reflection;

export class GameObject;

export
class ENGINE_CORE_API Component
{
	friend class GameObject;

public:
	virtual ~Component() = default;

	virtual void OnCreate() {}
	virtual void OnUpdate([[maybe_unused]] float dt) {}
	virtual void OnDestroy() {}

	// Returns the reflected field descriptors for this component instance.
	// Overridden by COMPONENT_BODY — returns an empty span for the base class.
	virtual std::span<const FieldDescriptor> GetReflectedFields() const { return {}; }

	GameObject* GetOwner() const { return _owner; }

	bool IsEnabled() const { return _enabled; }
	void SetEnabled(bool enabled) { _enabled = enabled; }

private:
	GameObject* _owner = nullptr;
	bool _enabled = true;
};
