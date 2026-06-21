module;

#include <cassert>

#include "engine_core_api.h"

export module Scene;

#if defined ( __INTELLISENSE__ )
#include "Component.ixx"
#include "IComponentStorage.ixx"
#include "GameObject.ixx"
#include "GUID.ixx"
#include "Interfaces.ixx"
#include "Math.ixx"
#include "Transform.ixx"
#else
import Component;
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

		struct SceneImpl
		{
			SceneImpl(IComponentStorage* componentStorage, 
				std::string_view, 
				std::size_t expectedNodeCount = 1024,
				unsigned long long rootGuid = 0);

			std::vector<Node> nodes;
			TransformStorage storage;
			std::deque<int> walkHelperQueue;
			std::string sceneName;
			// Declared last so it is destroyed first, ensuring components are torn down
			// (via ~GameObject -> DestroyComponent) before TransformStorage is gone.
			std::vector<std::unique_ptr<GameObject>> gameObjects;

			GameObject* CreateGameObject(IComponentStorage* componentStorage,
				std::string_view name,
				int parentNodeIndex = 0,
				unsigned long long guid = 0);

			void Clear();
		};

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
			
			void SetLocalTransform(int nodeIndex, const Mat4x4& localTransform);
			const Mat4x4& GetLocalTransform(int nodeIndex) const;
			void UpdateNodeIndex(int oldIndex, int newIndex);
			void SetParent(int nodeIndex, int newParentIndex);

			void UpdateWorldTransforms();
			void WalkDepthFirst(std::size_t startingNode,
				std::function<void(Scene&, std::size_t)> op);

			void WalkBreadthFirst(std::size_t startingNode,
				std::function<void(Scene&, std::size_t)> op);

			GameObject* CreateGameObject(IComponentStorage* storage,
				std::string_view name,
				int parentNodeIndex,
				unsigned long long guid = 0);

			GameObject* GetGameObject(std::size_t nodeIndex);
			const std::vector<std::unique_ptr<GameObject>>& GetAllGameObjects() const;

			const Engine::TransformStorage* GetTransformStorage() const;
			void Clear();

		private:
			// ScenePimpl; were I to use a direct member, or a unique_ptr, it would expose
			// SceneImpl to callers across dll boundaries, so I have to use a raw pointer to avoid
			// warning C4251. (Internally, SceneImpl uses TransformStorage which is _not_ exported,
			// so that would be a problem.)
			SceneImpl* _impl{ nullptr };

			void WalkBFSImpl(std::function<void(Scene&, std::size_t)> op);
			void UpdateDepthsBelow(int nodeIndex, int newDepth);
		};
	
	} // namespace Scene
} // namespace Engine