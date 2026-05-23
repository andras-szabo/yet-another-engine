module;

#include <cassert>
#include <deque>
#include <functional>
#include <string>
#include <vector>

export module SceneImpl;

import Component;
import Interfaces;
import Math;
import Transform;

namespace Engine
{
	namespace Scene
	{
		export struct Hierarchy
		{
			Hierarchy(int parent_, int depth_) : parent{ parent_ }, depth{ depth_ } {}

			int parent{ -1 };
			int firstChild{ -1 };
			int firstSibling{ -1 };
			int depth{ -1 };
			bool isDirty{ false };
		};

		export struct Node
		{
			int material{ -1 };
			int mesh{ -1 };
		};

		//TODO cleanup
		//class Transform;

		export class SceneImpl
		{
		public:
			SceneImpl(std::string_view name, std::size_t expectedNodeCount = 1024);

			std::string_view GetNodeName(std::size_t nodeIndex) const;
			int AddNode(const Mat4x4& localTransform, int parent, 
				const std::string& name, Transform* transformComponent = nullptr);

			void SetLocalTransform(int nodeIndex, const Mat4x4& localTransform);
			const Mat4x4& GetLocalTransform(int nodeIndex) const;
			void UpdateNodeIndex(int oldIndex, int newIndex);
			void SetParent(int nodeIndex, int newParentIndex);

			void UpdateWorldTransforms();
			void WalkDepthFirst(std::size_t startingNode, std::function<void(std::size_t)> op);
			void WalkBreadthFirst(std::size_t startingNode, std::function<void(std::size_t)> op);

			TransformStorage* GetTransformStorage();

		private:
			std::vector<Node> _nodes;
			TransformStorage _storage;
			//std::vector<Hierarchy> hierarchies;
			//std::vector<Mat4x4> globalTransforms;
			//std::vector<Mat4x4> localTransforms;
			//std::vector<std::string> _nodeNames;
			//std::vector<ISceneNodeIndexObserver*> _nodeTransformComponents;	// parallel vector: component associated with node i (or nullptr)

			std::deque<int> _walkHelperQueue;

			void WalkBFSImpl(std::function<void(std::size_t)> op);
			void UpdateDepthsBelow(int nodeIndex, int newDepth);
		};

		SceneImpl::SceneImpl(std::string_view name, std::size_t expectedNodeCount)
			: _storage{ expectedNodeCount , name }
		{
			_nodes.reserve(expectedNodeCount);
			_nodes.push_back(Node{});

			/*hierarchies.reserve(expectedNodeCount);
			globalTransforms.reserve(expectedNodeCount);
			localTransforms.reserve(expectedNodeCount);
			_nodeNames.reserve(expectedNodeCount);
			_nodeTransformComponents.reserve(expectedNodeCount);*/

			/*hierarchies.push_back(Hierarchy{ -1, 0 });
			globalTransforms.push_back(Mat4x4::Identity());
			localTransforms.push_back(Mat4x4::Identity());
			_nodeNames.push_back(std::string(name));
			_nodeTransformComponents.push_back(nullptr);*/
		}

		TransformStorage* SceneImpl::GetTransformStorage()
		{
			return &_storage;
		}

		void SceneImpl::UpdateWorldTransforms()
		{
			_storage.UpdateWorldTransforms();
			//WalkDepthFirst(0, [&](std::size_t nodeIndex)
			//	{
			//		if (hierarchies[nodeIndex].isDirty)
			//		{
			//			const auto parent = hierarchies[nodeIndex].parent;
			//			if (parent == -1)
			//			{
			//				globalTransforms[nodeIndex] = localTransforms[nodeIndex];
			//			}
			//			else
			//			{
			//				globalTransforms[nodeIndex] = globalTransforms[parent] * localTransforms[nodeIndex];
			//			}
			//			hierarchies[nodeIndex].isDirty = false;
			//		}
			//	});
		}

		std::string_view SceneImpl::GetNodeName(std::size_t nodeIndex) const
		{
			assert(!_storage.names.empty() && "Uninitialized scene!");
			return _storage.names[nodeIndex];

			//assert(!_nodeNames.empty() && "Uninitialized scene!");
			//return _nodeNames[nodeIndex];
		}

		int SceneImpl::AddNode(const Mat4x4& localTransform,
			int parent,
			const std::string& name,
			Transform* transformComponent)
		{
			_nodes.push_back(Node{});
			return _storage.AddTransform(localTransform, parent, name, transformComponent);
			/*assert(0 <= parent && parent < (int)_nodes.size() && "Specified parent invalid; it should be a non-negative scene node index. (0 = scene root)");

			const int newNodeIndex = static_cast<int>(std::size(_nodes));

			assert(newNodeIndex == (int)hierarchies.size() &&
				newNodeIndex == (int)globalTransforms.size() &&
				newNodeIndex == (int)localTransforms.size() &&
				newNodeIndex == (int)_nodeNames.size() &&
				newNodeIndex == (int)_nodeTransformComponents.size() && "Scene graph vectors out of sync!");

			// Actually, before pushing back, let's calculate all the things.
			// 1.) Find the parent.
			// 2.) If the parent's first child is -1, then replace it with current newNodeIndex, and Bob's your uncle.
			// 3.) If the parent's first child is not -1, then go there
			//		find child's first sibling.
			//			if child's first sibling is -1, then replace it, and Bob's your uncle.
			//			otherwise, continue going for siblings until you find a -1, and overwrite _that_.
			//				Bob's your uncle.

			auto& parentH = hierarchies[parent];
			int parentDepth = parentH.depth;
			if (parentH.firstChild == -1)
			{
				parentH.firstChild = newNodeIndex;
			}
			else
			{
				int nextNodeToCheck = parentH.firstChild;
				while (hierarchies[nextNodeToCheck].firstSibling != -1)
				{
					nextNodeToCheck = hierarchies[nextNodeToCheck].firstSibling;
				}
				hierarchies[nextNodeToCheck].firstSibling = newNodeIndex;
			}

			_nodes.push_back(Node{});
			hierarchies.push_back(Hierarchy{ parent, parentDepth + 1 });
			globalTransforms.push_back(globalTransforms[parent] * localTransform);
			localTransforms.push_back(localTransform);
			_nodeNames.push_back(name);
			_nodeTransformComponents.push_back(transformComponent);

			return newNodeIndex;*/
		};

		void SceneImpl::WalkDepthFirst(std::size_t startingNode, std::function<void(std::size_t currentNodeIndex)> op)
		{
			op(startingNode);

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

		void SceneImpl::WalkBreadthFirst(std::size_t startingNode, 
			std::function<void(std::size_t currentNodeIndex)> op)
		{
			_walkHelperQueue.clear();
			_walkHelperQueue.push_back(static_cast<int>(startingNode));

			WalkBFSImpl(op);
		}

		void SceneImpl::WalkBFSImpl(std::function<void(std::size_t currentNodeIndex)> op)
		{
			while (!_walkHelperQueue.empty())
			{
				const auto nextNode = _walkHelperQueue.front();
				_walkHelperQueue.pop_front();

				op(nextNode);

				const auto& node = _storage.hierarchies[nextNode];
				if (node.firstChild != -1)
				{
					_walkHelperQueue.push_back(node.firstChild);
				}

				auto nextSibling = node.firstSibling;
				while (nextSibling != -1)
				{
					op(nextSibling);

					const auto siblingsFirstChild = _storage.hierarchies[nextSibling].firstChild;
					if (siblingsFirstChild != -1)
					{
						_walkHelperQueue.push_back(siblingsFirstChild);
					}

					nextSibling = _storage.hierarchies[nextSibling].firstSibling;
				}
			}
		}

		void SceneImpl::SetLocalTransform(int nodeIndex, const Mat4x4& localTransform)
		{
			assert(nodeIndex >= 0 && nodeIndex < (int)_storage.localTransforms.size() && 
				"Node index out of range");

			_storage.localTransforms[nodeIndex] = localTransform;
			_storage.hierarchies[nodeIndex].isDirty = true;
		}

		const Mat4x4& SceneImpl::GetLocalTransform(int nodeIndex) const
		{
			assert(nodeIndex >= 0 && nodeIndex < (int)_storage.localTransforms.size() && 
				"Node index out of range");
			return _storage.localTransforms[nodeIndex];
		}

		void SceneImpl::UpdateNodeIndex(int oldIndex, int newIndex)
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

		void SceneImpl::SetParent(int nodeIndex, int newParentIndex)
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

		void SceneImpl::UpdateDepthsBelow(int nodeIndex, int newDepth)
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