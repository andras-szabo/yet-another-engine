module;

#include <span>

#include "engine_core_api.h"

export module Component;

import Reflection;
import Utility;

namespace Engine
{
	export class GameObject;

	export class ENGINE_CORE_API Component
	{
		friend class GameObject;

	public:
		virtual ~Component() = default;

		virtual void OnCreate() {}
		virtual void OnUpdate([[maybe_unused]] float dt) {}
		virtual void OnDestroy() {}
		virtual void OnSceneNodeIndexChanged([[maybe_unused]] int newIndex) {}

		// Returns the reflected field descriptors for this component instance.
		// Overridden by COMPONENT_BODY — returns an empty span for the base class.
		virtual std::span<const FieldDescriptor> GetReflectedFields() const { return {}; }

		GameObject* GetOwner() const { return _owner; }

		bool IsEnabled() const { return _enabled; }
		void SetEnabled(bool enabled) { _enabled = enabled; }

		constexpr virtual unsigned int GetTypeID() const { return 0; }

	private:
		GameObject* _owner = nullptr;
		bool _enabled = true;
	};

}

