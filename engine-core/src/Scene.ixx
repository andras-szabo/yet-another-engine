module;

#include <cassert>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "engine_core_api.h"

export module Scene;

import Component;
import IComponentStorage;
import GameObject;
import Interfaces;
import Math;
import Transform;

namespace Engine
{
	namespace Scene
	{
		export struct Node
		{
			int material{ -1 };
			int mesh{ -1 };
		};

		class Scene;

		struct SceneImpl
		{
			SceneImpl(IComponentStorage* componentStorage, std::string_view, std::size_t expectedNodeCount = 1024);

			std::vector<Node> nodes;
			std::vector<std::unique_ptr<GameObject>> gameObjects;
			TransformStorage storage;
			std::deque<int> walkHelperQueue;

			GameObject* CreateGameObject(IComponentStorage* componentStorage,
				std::string_view name,
				int parentNodeIndex = 0);
		};

		export class ENGINE_CORE_API Scene
		{
		public:
			Scene(IComponentStorage* componentStorage, std::string_view name, std::size_t expectedNodeCount = 1024);

			std::string_view GetName() const;
			std::string_view GetNodeName(std::size_t nodeIndex) const;

			//int AddNode(const Mat4x4& localTransform, int parent,
			//		const std::string& name, Transform* transformComponent = nullptr);

			void SetLocalTransform(int nodeIndex, const Mat4x4& localTransform);
			const Mat4x4& GetLocalTransform(int nodeIndex) const;
			void UpdateNodeIndex(int oldIndex, int newIndex);
			void SetParent(int nodeIndex, int newParentIndex);

			void UpdateWorldTransforms();
			void WalkDepthFirst(std::size_t startingNode,
				std::function<void(Scene&, std::size_t)> op);

			void WalkBreadthFirst(std::size_t startingNode,
				std::function<void(Scene&, std::size_t)> op);

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

		SceneImpl::SceneImpl(IComponentStorage* componentStorage,
			std::string_view sceneName, 
			std::size_t expectedNodeCount)
			: storage{ expectedNodeCount }
		{
			nodes.reserve(expectedNodeCount);
			nodes.push_back(Node{});

			gameObjects.reserve(expectedNodeCount);

			const std::string rootName = std::string{ sceneName } + "_root";
			CreateGameObject(componentStorage, rootName, -1);
		}

		GameObject* SceneImpl::CreateGameObject(IComponentStorage* componentStorage,
			std::string_view name,
			int parentNodeIndex)
		{
			auto go = std::make_unique<GameObject>(name);
			go->AddComponent<Engine::Transform>(componentStorage, &storage, name, parentNodeIndex);

			return go.get();
		}

		Scene::Scene(IComponentStorage* componentStorage, std::string_view name, std::size_t expectedNodeCount)
			: _impl{ new SceneImpl(componentStorage, name, expectedNodeCount) }
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

		std::string_view Scene::GetName() const
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