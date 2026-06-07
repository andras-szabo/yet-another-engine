module;

#include <cassert>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <vector>

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
			std::vector<std::unique_ptr<GameObject>> gameObjects;
			TransformStorage storage;
			std::deque<int> walkHelperQueue;

			GameObject* CreateGameObject(IComponentStorage* componentStorage,
				std::string_view name,
				int parentNodeIndex = 0,
				unsigned long long guid = 0);
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
			std::vector<std::unique_ptr<GameObject>>& GetAllGameObjects();

			TransformStorage* GetTransformStorage();

		private:
			// ScenePimpl; were I to use a direct member, or a unique_ptr, it would expose
			// SceneImpl to callers across dll boundaries, so I have to use a raw pointer to avoid
			// warning C4251. (Internally, SceneImpl uses TransformStorage which is _not_ exported,
			// so that would be a problem.)
			SceneImpl* _impl{ nullptr };

			void WalkBFSImpl(std::function<void(Scene&, std::size_t)> op);
			void UpdateDepthsBelow(int nodeIndex, int newDepth);
		};

		Scene::Scene(Scene&& other) noexcept
			: _impl{ other._impl }
		{
			other._impl = nullptr;
		}

		Scene& Scene::operator=(Scene&& other) noexcept
		{
			if (this != &other)
			{
				delete _impl;
				_impl = other._impl;
				other._impl = nullptr;
			}

			return *this;
		}

		Scene::~Scene()
		{
			delete _impl;
		}

		Engine::GameObject* Scene::GetGameObject(std::size_t nodeIndex)
		{
			assert(nodeIndex < _impl->gameObjects.size() && "Node index out of bounds.");
			return _impl->gameObjects[nodeIndex].get();
		}

		std::vector<std::unique_ptr<GameObject>>& Scene::GetAllGameObjects()
		{
			return _impl->gameObjects;
		}

		SceneImpl::SceneImpl(IComponentStorage* componentStorage,
			std::string_view sceneName, 
			std::size_t expectedNodeCount,
			unsigned long long rootGuid)
			: storage{ expectedNodeCount }
		{
			nodes.reserve(expectedNodeCount);
			gameObjects.reserve(expectedNodeCount);

			const std::string rootName = std::string{ sceneName } + "_root";
			if (rootGuid == 0)
			{
				rootGuid = GUID().id;
			}
			CreateGameObject(componentStorage, rootName, -1, rootGuid);
		}

		GameObject* Scene::CreateGameObject(IComponentStorage* componentStorage,
			std::string_view name,
			int parentNodeIndex,
			unsigned long long guid)
		{
			if (guid == 0)
			{
				guid = Engine::GUID().id;
			}

			return _impl->CreateGameObject(componentStorage, name, parentNodeIndex, guid);
		}


		GameObject* SceneImpl::CreateGameObject(IComponentStorage* componentStorage,
			std::string_view name,
			int parentNodeIndex,
			unsigned long long guid)
		{
			auto go = std::make_unique<GameObject>(name, guid);
			auto transform = go->AddComponent<Engine::Transform>(componentStorage, &storage, name, parentNodeIndex);
			go->SetTransform(transform);

			gameObjects.push_back(std::move(go));
			nodes.push_back(Node{});

			return (*gameObjects.rbegin()).get();
		}

		Scene::Scene(IComponentStorage* componentStorage, 
			std::string_view name, 
			std::size_t expectedNodeCount,
			unsigned long long rootGuid)
			: _impl{ new SceneImpl(componentStorage, name, expectedNodeCount, rootGuid) }
		{
		}

		TransformStorage* Scene::GetTransformStorage()
		{
			return &(*_impl).storage;
		}

		void Scene::UpdateWorldTransforms()
		{
			_impl->storage.UpdateWorldTransforms();
		}

		std::string_view Scene::GetSceneName() const
		{
			return GetNodeName(0);
		}

		std::string_view Scene::GetNodeName(std::size_t nodeIndex) const
		{
			assert(!_impl->storage.names.empty() && "Uninitialized scene!");
			return _impl->storage.names[nodeIndex];
		}

		//int Scene::AddNode(const Mat4x4& localTransform,
		//	int parent,
		//	const std::string& name,
		//	Transform* transformComponent)
		//{
		//	_impl->nodes.push_back(Node{});
		//	return _impl->storage.AddTransform(localTransform, parent, name, transformComponent);
		//};

		void Scene::WalkDepthFirst(std::size_t startingNode,
			std::function<void(Scene& scene, std::size_t currentNodeIndex)> op)
		{
			op(*this, startingNode);

			const auto node = _impl->storage.hierarchies[startingNode];
			if (node.firstChild != -1)
			{
				WalkDepthFirst(node.firstChild, op);
			}

			int nextSibling = node.firstSibling;
			if (nextSibling != -1)
			{
				WalkDepthFirst(nextSibling, op);
			}
		}

		void Scene::WalkBreadthFirst(std::size_t startingNode,
			std::function<void(Scene& scene, std::size_t currentNodeIndex)> op)
		{
			_impl->walkHelperQueue.clear();
			_impl->walkHelperQueue.push_back(static_cast<int>(startingNode));

			WalkBFSImpl(op);
		}

		void Scene::WalkBFSImpl(std::function<void(
			Scene& scene,
			std::size_t currentNodeIndex)> op)
		{
			while (!_impl->walkHelperQueue.empty())
			{
				const auto nextNode = _impl->walkHelperQueue.front();
				_impl->walkHelperQueue.pop_front();

				op(*this, nextNode);

				const auto& node = _impl->storage.hierarchies[nextNode];
				if (node.firstChild != -1)
				{
					_impl->walkHelperQueue.push_back(node.firstChild);
				}

				auto nextSibling = node.firstSibling;
				while (nextSibling != -1)
				{
					op(*this, nextSibling);

					const auto siblingsFirstChild = _impl->storage.hierarchies[nextSibling].firstChild;
					if (siblingsFirstChild != -1)
					{
						_impl->walkHelperQueue.push_back(siblingsFirstChild);
					}

					nextSibling = _impl->storage.hierarchies[nextSibling].firstSibling;
				}
			}
		}

		void Scene::SetLocalTransform(int nodeIndex, const Mat4x4& localTransform)
		{
			assert(nodeIndex >= 0 && nodeIndex < (int)_impl->storage.localTransforms.size() &&
				"Node index out of range");

			_impl->storage.localTransforms[nodeIndex] = localTransform;
			_impl->storage.hierarchies[nodeIndex].isDirty = true;
		}

		const Mat4x4& Scene::GetLocalTransform(int nodeIndex) const
		{
			assert(nodeIndex >= 0 && nodeIndex < (int)_impl->storage.localTransforms.size() &&
				"Node index out of range");
			return _impl->storage.localTransforms[nodeIndex];
		}

		void Scene::UpdateNodeIndex(int oldIndex, int newIndex)
		{
			assert(oldIndex >= 0 && oldIndex < (int)_impl->storage.transformComponents.size() && "Old node index out of range");
			assert(newIndex >= 0 && newIndex < (int)_impl->storage.transformComponents.size() && "New node index out of range");

			auto* transform = _impl->storage.transformComponents[oldIndex];
			_impl->storage.transformComponents[oldIndex] = nullptr;
			_impl->storage.transformComponents[newIndex] = transform;

			if (transform != nullptr)
			{
				transform->OnSceneNodeIndexChanged(newIndex);
			}
		}

		void Scene::SetParent(int nodeIndex, int newParentIndex)
		{
			assert(nodeIndex > 0 && "Cannot reparent the root node");
			assert(nodeIndex != newParentIndex && "Cannot set a node as its own parent");
			assert(newParentIndex >= 0 && newParentIndex < (int)_impl->nodes.size() && "New parent index is out of range");

			const int currentParent = _impl->storage.hierarchies[nodeIndex].parent;

			// Unlink from current parent's child/sibling chain
			if (_impl->storage.hierarchies[currentParent].firstChild == nodeIndex)
			{
				_impl->storage.hierarchies[currentParent].firstChild =
					_impl->storage.hierarchies[nodeIndex].firstSibling;
			}
			else
			{
				int prev = _impl->storage.hierarchies[currentParent].firstChild;
				while (_impl->storage.hierarchies[prev].firstSibling != nodeIndex)
				{
					prev = _impl->storage.hierarchies[prev].firstSibling;
					assert(prev != -1 && "Node not found in parent's sibling chain");
				}

				_impl->storage.hierarchies[prev].firstSibling =
					_impl->storage.hierarchies[nodeIndex].firstSibling;
			}

			// Clear old sibling link before re-linking
			_impl->storage.hierarchies[nodeIndex].firstSibling = -1;

			// Append as last child of the new parent
			if (_impl->storage.hierarchies[newParentIndex].firstChild == -1)
			{
				_impl->storage.hierarchies[newParentIndex].firstChild = nodeIndex;
			}
			else
			{
				int lastChild = _impl->storage.hierarchies[newParentIndex].firstChild;
				while (_impl->storage.hierarchies[lastChild].firstSibling != -1)
				{
					lastChild = _impl->storage.hierarchies[lastChild].firstSibling;
				}
				_impl->storage.hierarchies[lastChild].firstSibling = nodeIndex;
			}

			_impl->storage.hierarchies[nodeIndex].parent = newParentIndex;
			UpdateDepthsBelow(nodeIndex, _impl->storage.hierarchies[newParentIndex].depth + 1);
			_impl->storage.hierarchies[nodeIndex].isDirty = true;
		}

		void Scene::UpdateDepthsBelow(int nodeIndex, int newDepth)
		{
			_impl->storage.hierarchies[nodeIndex].depth = newDepth;
			int child = _impl->storage.hierarchies[nodeIndex].firstChild;
			while (child != -1)
			{
				UpdateDepthsBelow(child, newDepth + 1);
				child = _impl->storage.hierarchies[child].firstSibling;
			}
		}
	} // namespace Scene
} // namespace Engine