module;

#include "engine_core_api.h"

export module Interfaces;

namespace Engine
{
	export class ENGINE_CORE_API ISceneNodeIndexObserver
	{
	public:
		virtual ~ISceneNodeIndexObserver() = default;
		virtual void OnSceneNodeIndexChanged(int newIndex) = 0;
	};
} // namespace Engine
