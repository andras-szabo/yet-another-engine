module;

#include <cassert>
#include <functional>
#include <string>
#include <queue>
#include <vector>

#include "engine_core_api.h"
#include "LoggerMacros.h"

export module Scene;

import Logger;
import Math;

namespace Engine
{
	namespace Scene
	{
		export struct ENGINE_CORE_API Hierarchy
		{
			Hierarchy(int parent_, int depth_) : parent{ parent_ }, depth{ depth_ } {}

			int parent{ -1 };
			int firstChild{ -1 };
			int firstSibling{ -1 };
			int depth{ -1 };
			bool isDirty{ false };
		};

		export struct ENGINE_CORE_API Node
		{
			int material{ -1 };
			int mesh{ -1 };
		};

		export class ENGINE_CORE_API Scene
		{
		public:
			Scene(std::string_view name, std::size_t expectedNodeCount = 1024);
			int AddNode(const Mat4x4& localTransform, int parent, const std::string& name);

			std::string_view GetName() const;
			int GetRootIndex() const;
			std::string_view GetNodeName(std::size_t index) const;

			void WalkDepthFirst(std::size_t startingNode, std::function<void(std::size_t)> op);
			void WalkBreadthFirst(std::size_t startingNode, std::function<void(std::size_t)> op);

		private:
			std::vector<Node> _nodes;
			std::vector<Hierarchy> _hierarchy;
			std::vector<Mat4x4> _globalTransforms;
			std::vector<Mat4x4> _localTransforms;

			std::vector<std::string> _nodeNames;

			std::deque<int> _walkHelperQueueContainer;

			void WalkBFSImpl(std::queue<int, std::deque<int>>& nodesToVisit, std::function<void(std::size_t)> op);
		};

		Scene::Scene(std::string_view name, std::size_t expectedNodeCount)
		{
			_nodes.reserve(expectedNodeCount);
			_hierarchy.reserve(expectedNodeCount);
			_globalTransforms.reserve(expectedNodeCount);
			_localTransforms.reserve(expectedNodeCount);
			_nodeNames.reserve(expectedNodeCount);

			_nodes.push_back(Node{});
			_hierarchy.push_back(Hierarchy{ -1, 0 });
			_globalTransforms.push_back(Mat4x4::Identity());
			_localTransforms.push_back(Mat4x4::Identity());
			_nodeNames.push_back(std::string(name));
		}

		int Scene::GetRootIndex() const { return 0; }
		std::string_view Scene::GetNodeName(std::size_t nodeIndex) const { return _nodeNames[nodeIndex]; }

		std::string_view Scene::GetName() const
		{
			assert(!_nodeNames.empty() && "Scene has no root constructed.");
			return _nodeNames[0];
		}

		int Scene::AddNode(const Mat4x4& localTransform, int parent, const std::string& name)
		{
			assert(0 <= parent && parent < _nodes.size() && "Specified parent invalid; it should be a non-negative scene node index. (0 = scene root)");

			const int newNodeIndex = std::size(_nodes);

			assert(newNodeIndex == _hierarchy.size() &&
				newNodeIndex == _globalTransforms.size() &&
				newNodeIndex == _localTransforms.size() &&
				newNodeIndex == _nodeNames.size() && "Scene graph vectors out of sync!");

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

			Mat4x4 globalTransform = _globalTransforms[parent] * localTransform;

			_nodes.push_back(Node{});
			_hierarchy.push_back(Hierarchy{ parent, parentDepth + 1 });
			_globalTransforms.push_back(_globalTransforms[parent] * localTransform);
			_localTransforms.push_back(localTransform);
			_nodeNames.push_back(name);

			return newNodeIndex;
		};

		void Scene::WalkDepthFirst(std::size_t startingNode, std::function<void(std::size_t currentNodeIndex)> op)
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

		void Scene::WalkBreadthFirst(std::size_t startingNode, std::function<void(std::size_t currentNodeIndex)> op)
		{
			_walkHelperQueueContainer.clear();
			std::queue<int, std::deque<int>> nodesToVisit{ _walkHelperQueueContainer };
			nodesToVisit.push(startingNode);

			WalkBFSImpl(nodesToVisit, op);
		}

		void Scene::WalkBFSImpl(std::queue<int, std::deque<int>>& nodesToVisit, std::function<void(std::size_t)> op)
		{
			while (!nodesToVisit.empty())
			{
				const auto nextNode = nodesToVisit.front();
				nodesToVisit.pop();

				op(nextNode);

				const auto& node = _hierarchy[nextNode];
				if (node.firstChild != -1)
				{
					nodesToVisit.push(node.firstChild);
				}

				auto nextSibling = node.firstSibling;
				while (nextSibling != -1)
				{
					op(nextSibling);

					const auto siblingsFirstChild = _hierarchy[nextSibling].firstChild;
					if (siblingsFirstChild != -1)
					{
						nodesToVisit.push(siblingsFirstChild);
					}

					nextSibling = _hierarchy[nextSibling].firstSibling;
				}
			}
		}
	} // namespace Scene
} // namespace Engine