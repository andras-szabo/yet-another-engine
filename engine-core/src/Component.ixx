module;

#include "engine_core_api.h"

export module Component;

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

	GameObject* GetOwner() const { return _owner; }

	bool IsEnabled() const { return _enabled; }
	void SetEnabled(bool enabled) { _enabled = enabled; }

private:
	GameObject* _owner = nullptr;
	bool _enabled = true;
};
