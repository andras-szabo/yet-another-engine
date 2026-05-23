module;

#include <cassert>
#include <deque>
#include <functional>
#include <string>
#include <vector>

#include "engine_core_api.h"

export module Scene;

import Component;
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

		export class ENGINE_CORE_API Scene
		{
		public:
			Scene();
			Scene(std::string_view name, std::size_t expectedNodeCount = 1024);

			std::string_view GetName() const;
			std::string_view GetNodeName(std::size_t nodeIndex) const;

			int AddNode(const Mat4x4& localTransform, int parent, 
				const std::string& name, Transform* transformComponent = nullptr);

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
			std::vector<Node> _nodes;
			TransformStorage _storage;
			std::deque<int> _walkHelperQueue;

			void WalkBFSImpl(std::function<void(Scene&, std::size_t)> op);
			void UpdateDepthsBelow(int nodeIndex, int newDepth);
		};

		Scene::Scene() : Scene("NewScene", 1024)
		{
		}

		Scene::Scene(std::string_view name, std::size_t expectedNodeCount)
			: _storage{ expectedNodeCount , name }
		{
			_nodes.reserve(expectedNodeCount);
			_nodes.push_back(Node{});
		}

		TransformStorage* Scene::GetTransformStorage()
		{
			return &_storage;
		}

		void Scene::UpdateWorldTransforms()
		{
			_storage.UpdateWorldTransforms();
		}

		std::string_view Scene::GetName() const
		{
			return GetNodeName(0);
		}

		std::string_view Scene::GetNodeName(std::size_t nodeIndex) const
		{
			assert(!_storage.names.empty() && "Uninitialized scene!");
			return _storage.names[nodeIndex];
		}

		int Scene::AddNode(const Mat4x4& localTransform,
			int parent,
			const std::string& name,
			Transform* transformComponent)
		{
			_nodes.push_back(Node{});
			return _storage.AddTransform(localTransform, parent, name, transformComponent);
		};

		void Scene::WalkDepthFirst(std::size_t startingNode, 
			std::function<void(Scene& scene, std::size_t currentNodeIndex)> op)
		{
			op(*this, startingNode);

			const auto node = _storage.hierarchies[startingNode];
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
			_walkHelperQueue.clear();
			_walkHelperQueue.push_back(static_cast<int>(startingNode));

			WalkBFSImpl(op);
		}

		void Scene::WalkBFSImpl(std::function<void(
			Scene& scene,
			std::size_t currentNodeIndex)> op)
		{
			while (!_walkHelperQueue.empty())
			{
				const auto nextNode = _walkHelperQueue.front();
				_walkHelperQueue.pop_front();

				op(*this, nextNode);

				const auto& node = _storage.hierarchies[nextNode];
				if (node.firstChild != -1)
				{
					_walkHelperQueue.push_back(node.firstChild);
				}

				auto nextSibling = node.firstSibling;
				while (nextSibling != -1)
				{
					op(*this, nextSibling);

					const auto siblingsFirstChild = _storage.hierarchies[nextSibling].firstChild;
					if (siblingsFirstChild != -1)
					{
						_walkHelperQueue.push_back(siblingsFirstChild);
					}

					nextSibling = _storage.hierarchies[nextSibling].firstSibling;
				}
			}
		}

		void Scene::SetLocalTransform(int nodeIndex, const Mat4x4& localTransform)
		{
			assert(nodeIndex >= 0 && nodeIndex < (int)_storage.localTransforms.size() && 
				"Node index out of range");

			_storage.localTransforms[nodeIndex] = localTransform;
			_storage.hierarchies[nodeIndex].isDirty = true;
		}

		const Mat4x4& Scene::GetLocalTransform(int nodeIndex) const
		{
			assert(nodeIndex >= 0 && nodeIndex < (int)_storage.localTransforms.size() && 
				"Node index out of range");
			return _storage.localTransforms[nodeIndex];
		}

		void Scene::UpdateNodeIndex(int oldIndex, int newIndex)
		{
			assert(oldIndex >= 0 && oldIndex < (int)_storage.transformComponents.size() && "Old node index out of range");
			assert(newIndex >= 0 && newIndex < (int)_storage.transformComponents.size() && "New node index out of range");

			auto* transform = _storage.transformComponents[oldIndex];
			_storage.transformComponents[oldIndex] = nullptr;
			_storage.transformComponents[newIndex] = transform;

			if (transform != nullptr)
			{
				transform->OnSceneNodeIndexChanged(newIndex);
			}
		}

		void Scene::SetParent(int nodeIndex, int newParentIndex)
		{
			assert(nodeIndex > 0 && "Cannot reparent the root node");
			assert(nodeIndex != newParentIndex && "Cannot set a node as its own parent");
			assert(newParentIndex >= 0 && newParentIndex < (int)_nodes.size() && "New parent index is out of range");

			const int currentParent = _storage.hierarchies[nodeIndex].parent;

			// Unlink from current parent's child/sibling chain
			if (_storage.hierarchies[currentParent].firstChild == nodeIndex)
			{
				_storage.hierarchies[currentParent].firstChild = 
					_storage.hierarchies[nodeIndex].firstSibling;
			}
			else
			{
				int prev = _storage.hierarchies[currentParent].firstChild;
				while (_storage.hierarchies[prev].firstSibling != nodeIndex)
				{
					prev = _storage.hierarchies[prev].firstSibling;
					assert(prev != -1 && "Node not found in parent's sibling chain");
				}

				_storage.hierarchies[prev].firstSibling = 
					_storage.hierarchies[nodeIndex].firstSibling;
			}

			// Clear old sibling link before re-linking
			_storage.hierarchies[nodeIndex].firstSibling = -1;

			// Append as last child of the new parent
			if (_storage.hierarchies[newParentIndex].firstChild == -1)
			{
				_storage.hierarchies[newParentIndex].firstChild = nodeIndex;
			}
			else
			{
				int lastChild = _storage.hierarchies[newParentIndex].firstChild;
				while (_storage.hierarchies[lastChild].firstSibling != -1)
				{
					lastChild = _storage.hierarchies[lastChild].firstSibling;
				}
				_storage.hierarchies[lastChild].firstSibling = nodeIndex;
			}

			_storage.hierarchies[nodeIndex].parent = newParentIndex;
			UpdateDepthsBelow(nodeIndex, _storage.hierarchies[newParentIndex].depth + 1);
			_storage.hierarchies[nodeIndex].isDirty = true;
		}

		void Scene::UpdateDepthsBelow(int nodeIndex, int newDepth)
		{
			_storage.hierarchies[nodeIndex].depth = newDepth;
			int child = _storage.hierarchies[nodeIndex].firstChild;
			while (child != -1)
			{
				UpdateDepthsBelow(child, newDepth + 1);
				child = _storage.hierarchies[child].firstSibling;
			}
		}
	} // namespace Scene
} // namespace Engine