module;

#include <cassert>

#include "engine_core_api.h"

export module Scene;

#if defined ( __INTELLISENSE__ )
#include "Component.ixx"
#include "EngineError.ixx"
#include "IComponentStorage.ixx"
#include "GameObject.ixx"
#include "GUID.ixx"
#include "Interfaces.ixx"
#include "Math.ixx"
#include "Transform.ixx"
#else
import Component;
import Error;
import IComponentStorage;
import GameObject;
import GUID;
import Interfaces;
import Math;
import Transform;

import std;
#endif

namespace Engine
{
	namespace Scene
	{
		export struct Node
		{
			int material{ -1 };
			int mesh{ -1 };
		};

		struct SceneImpl;

		export class ENGINE_CORE_API Scene
		{
		public:
			Scene(IComponentStorage* componentStorage, 
				std::string_view name, 
				std::size_t expectedNodeCount = 1024,
				unsigned long long rootGuid = 0);

			~Scene();

			Scene(const Scene&)			   = delete;
			Scene& operator=(const Scene&) = delete;
			Scene(Scene&& other) noexcept;
			Scene& operator=(Scene&& other) noexcept;

			std::string_view GetSceneName() const;
			std::string_view GetNodeName(std::size_t nodeIndex) const;
			GUID GetNodeGuid(std::size_t nodeIndex) const;
			void SetSceneName(std::string_view newName);
			Engine::Expected<void> SetNodeName(std::size_t nodeIndex, std::string_view newName);
			
			void SetLocalTransform(int nodeIndex, const Mat4x4& localTransform);
			const Mat4x4& GetLocalTransform(int nodeIndex) const;
			void UpdateNodeIndex(int oldIndex, int newIndex);
			void SetParent(int nodeIndex, int newParentIndex);

			void UpdateWorldTransforms();
			void WalkDepthFirst(std::size_t startingNode,
				std::function<void(Scene&, std::size_t, int)> op);

			void WalkBreadthFirst(std::size_t startingNode,
				std::function<void(Scene&, std::size_t, int)> op);

			GameObject* CreateGameObject(IComponentStorage* storage,
				std::string_view name,
				int parentNodeIndex,
				unsigned long long _guid = 0);

			int GetNodeCount() const;
			GameObject* GetGameObject(std::size_t nodeIndex);
			const GameObject* GetGameObject(std::size_t nodeIndex) const;
			const std::vector<std::unique_ptr<GameObject>>& GetAllGameObjects() const;

			const Engine::TransformStorage* GetTransformStorage() const;
			void Clear();

		private:
			// ScenePimpl; were I to use a direct member, or a unique_ptr, it would expose
			// SceneImpl to callers across dll boundaries, so I have to use a raw pointer to avoid
			// warning C4251. (Internally, SceneImpl uses TransformStorage which is _not_ exported,
			// so that would be a problem.)
			SceneImpl* _impl{ nullptr };

			void WalkBFSImpl(std::function<void(Scene&, std::size_t, int)> op);
			void UpdateDepthsBelow(int nodeIndex, int newDepth);
		};
	
	} // namespace Scene
} // namespace Engine