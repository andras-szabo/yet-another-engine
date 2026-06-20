module;

#include "engine_core_api.h"

export module Component;

#if defined ( __INTELLISENSE__ )
#include "Reflection.ixx"
#include "Utility.ixx"
#else
import Reflection;
import Utility;

import std;
#endif

namespace Engine
{
	export class ENGINE_CORE_API Component
	{
		friend class GameObject;

	public:
		virtual ~Component() = default;

		virtual void OnCreate() {}
		virtual void OnUpdate([[maybe_unused]] float dt) {}
		virtual void OnDestroy() {}

		virtual FieldSpan GetReflectedFields() const { return {}; }

		GameObject* GetOwner() const { return _owner; }

		bool IsEnabled() const { return _enabled; }
		void SetEnabled(bool enabled) { _enabled = enabled; }

		constexpr virtual unsigned int GetTypeID() const { return 0; }

	private:
		GameObject* _owner = nullptr;
		bool _enabled = true;
	};

}

