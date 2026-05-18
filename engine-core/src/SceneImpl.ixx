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

		class Transform;

		export class SceneImpl
		{
		public:
			SceneImpl(std::string_view name, std::size_t expectedNodeCount = 1024);

			std::string_view GetNodeName(std::size_t nodeIndex) const;
			int AddNode(const Mat4x4& localTransform, int parent, const std::string& name, ISceneNodeIndexObserver* transformComponent = nullptr);

			void SetLocalTransform(int nodeIndex, const Mat4x4& localTransform);
			const Mat4x4& GetLocalTransform(int nodeIndex) const;
			void UpdateNodeIndex(int oldIndex, int newIndex);
			void SetParent(int nodeIndex, int newParentIndex);

			void UpdateWorldTransforms();
			void WalkDepthFirst(std::size_t startingNode, std::function<void(std::size_t)> op);
			void WalkBreadthFirst(std::size_t startingNode, std::function<void(std::size_t)> op);

		private:
			std::vector<Node> _nodes;
			std::vector<Hierarchy> _hierarchy;
			std::vector<Mat4x4> _globalTransforms;
			std::vector<Mat4x4> _localTransforms;
			std::vector<std::string> _nodeNames;
			std::vector<ISceneNodeIndexObserver*> _nodeTransformComponents;	// parallel vector: component associated with node i (or nullptr)

			std::deque<int> _walkHelperQueue;

			void WalkBFSImpl(std::function<void(std::size_t)> op);
			void UpdateDepthsBelow(int nodeIndex, int newDepth);
		};

		SceneImpl::SceneImpl(std::string_view name, std::size_t expectedNodeCount)
		{
			_nodes.reserve(expectedNodeCount);
			_hierarchy.reserve(expectedNodeCount);
			_globalTransforms.reserve(expectedNodeCount);
			_localTransforms.reserve(expectedNodeCount);
			_nodeNames.reserve(expectedNodeCount);
			_nodeTransformComponents.reserve(expectedNodeCount);

			// Create the scene root
			_nodes.push_back(Node{});
			_hierarchy.push_back(Hierarchy{ -1, 0 });
			_globalTransforms.push_back(Mat4x4::Identity());
			_localTransforms.push_back(Mat4x4::Identity());
			_nodeNames.push_back(std::string(name));
			_nodeTransformComponents.push_back(nullptr);
		}

		void SceneImpl::UpdateWorldTransforms()
		{
			WalkDepthFirst(0, [&](std::size_t nodeIndex)
				{
					if (_hierarchy[nodeIndex].isDirty)
					{
						const auto parent = _hierarchy[nodeIndex].parent;
						if (parent == -1)
						{
							_globalTransforms[nodeIndex] = _localTransforms[nodeIndex];
						}
						else
						{
							_globalTransforms[nodeIndex] = _globalTransforms[parent] * _localTransforms[nodeIndex];
						}
						_hierarchy[nodeIndex].isDirty = false;
					}
				});
		}

		std::string_view SceneImpl::GetNodeName(std::size_t nodeIndex) const
		{
			assert(!_nodeNames.empty() && "Uninitialized scene!");
			return _nodeNames[nodeIndex];
		}

		int SceneImpl::AddNode(const Mat4x4& localTransform, int parent, const std::string& name, ISceneNodeIndexObserver* transformComponent)
		{
			assert(0 <= parent && parent < (int)_nodes.size() && "Specified parent invalid; it should be a non-negative scene node index. (0 = scene root)");

			const int newNodeIndex = static_cast<int>(std::size(_nodes));

			assert(newNodeIndex == (int)_hierarchy.size() &&
				newNodeIndex == (int)_globalTransforms.size() &&
				newNodeIndex == (int)_localTransforms.size() &&
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

			auto& parentH = _hierarchy[parent];
			int parentDepth = parentH.depth;
			if (parentH.firstChild == -1)
			{
				parentH.firstChild = newNodeIndex;
			}
			else
			{
				int nextNodeToCheck = parentH.firstChild;
				while (_hierarchy[nextNodeToCheck].firstSibling != -1)
				{
					nextNodeToCheck = _hierarchy[nextNodeToCheck].firstSibling;
				}
				_hierarchy[nextNodeToCheck].firstSibling = newNodeIndex;
			}

			_nodes.push_back(Node{});
			_hierarchy.push_back(Hierarchy{ parent, parentDepth + 1 });
			_globalTransforms.push_back(_globalTransforms[parent] * localTransform);
			_localTransforms.push_back(localTransform);
			_nodeNames.push_back(name);
			_nodeTransformComponents.push_back(transformComponent);

			return newNodeIndex;
		};

		void SceneImpl::WalkDepthFirst(std::size_t startingNode, std::function<void(std::size_t currentNodeIndex)> op)
		{
			op(startingNode);

			const auto node = _hierarchy[startingNode];
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

		void SceneImpl::WalkBreadthFirst(std::size_t startingNode, std::function<void(std::size_t currentNodeIndex)> op)
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

				const auto& node = _hierarchy[nextNode];
				if (node.firstChild != -1)
				{
					_walkHelperQueue.push_back(node.firstChild);
				}

				auto nextSibling = node.firstSibling;
				while (nextSibling != -1)
				{
					op(nextSibling);

					const auto siblingsFirstChild = _hierarchy[nextSibling].firstChild;
					if (siblingsFirstChild != -1)
					{
						_walkHelperQueue.push_back(siblingsFirstChild);
					}

					nextSibling = _hierarchy[nextSibling].firstSibling;
				}
			}
		}

		void SceneImpl::SetLocalTransform(int nodeIndex, const Mat4x4& localTransform)
		{
			assert(nodeIndex >= 0 && nodeIndex < (int)_localTransforms.size() && "Node index out of range");
			_localTransforms[nodeIndex] = localTransform;
			_hierarchy[nodeIndex].isDirty = true;
		}

		const Mat4x4& SceneImpl::GetLocalTransform(int nodeIndex) const
		{
			assert(nodeIndex >= 0 && nodeIndex < (int)_localTransforms.size() && "Node index out of range");
			return _localTransforms[nodeIndex];
		}

		void SceneImpl::UpdateNodeIndex(int oldIndex, int newIndex)
		{
			assert(oldIndex >= 0 && oldIndex < (int)_nodeTransformComponents.size() && "Old node index out of range");
			assert(newIndex >= 0 && newIndex < (int)_nodeTransformComponents.size() && "New node index out of range");

			auto* transform = _nodeTransformComponents[oldIndex];
			_nodeTransformComponents[oldIndex] = nullptr;
			_nodeTransformComponents[newIndex] = transform;

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

			const int currentParent = _hierarchy[nodeIndex].parent;

			// Unlink from current parent's child/sibling chain
			if (_hierarchy[currentParent].firstChild == nodeIndex)
			{
				_hierarchy[currentParent].firstChild = _hierarchy[nodeIndex].firstSibling;
			}
			else
			{
				int prev = _hierarchy[currentParent].firstChild;
				while (_hierarchy[prev].firstSibling != nodeIndex)
				{
					prev = _hierarchy[prev].firstSibling;
					assert(prev != -1 && "Node not found in parent's sibling chain");
				}
				_hierarchy[prev].firstSibling = _hierarchy[nodeIndex].firstSibling;
			}

			// Clear old sibling link before re-linking
			_hierarchy[nodeIndex].firstSibling = -1;

			// Append as last child of the new parent
			if (_hierarchy[newParentIndex].firstChild == -1)
			{
				_hierarchy[newParentIndex].firstChild = nodeIndex;
			}
			else
			{
				int lastChild = _hierarchy[newParentIndex].firstChild;
				while (_hierarchy[lastChild].firstSibling != -1)
				{
					lastChild = _hierarchy[lastChild].firstSibling;
				}
				_hierarchy[lastChild].firstSibling = nodeIndex;
			}

			_hierarchy[nodeIndex].parent = newParentIndex;
			UpdateDepthsBelow(nodeIndex, _hierarchy[newParentIndex].depth + 1);
			_hierarchy[nodeIndex].isDirty = true;
		}

		void SceneImpl::UpdateDepthsBelow(int nodeIndex, int newDepth)
		{
			_hierarchy[nodeIndex].depth = newDepth;
			int child = _hierarchy[nodeIndex].firstChild;
			while (child != -1)
			{
				UpdateDepthsBelow(child, newDepth + 1);
				child = _hierarchy[child].firstSibling;
			}
		}
	} // namespace Scene
} // namespace Engine