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

		class Scene;

		class SceneImpl
		{
		public:
			SceneImpl(std::string_view name, std::size_t expectedNodeCount = 1024);

			std::string_view GetNodeName(std::size_t nodeIndex) const;
			int AddNode(const Mat4x4& localTransform, int parent, const std::string& name);

			void UpdateWorldTransforms(Scene& scene);
			void WalkDepthFirst(Scene& scene, std::size_t startingNode, std::function<void(Scene&, std::size_t)> op);
			void WalkBreadthFirst(Scene& scene, std::size_t startingNode, std::function<void(Scene&, std::size_t)> op);

		private:
			std::vector<Node> _nodes;
			std::vector<Hierarchy> _hierarchy;
			std::vector<Mat4x4> _globalTransforms;
			std::vector<Mat4x4> _localTransforms;
			std::vector<std::string> _nodeNames;

			std::deque<int> _walkHelperQueue;

			void WalkBFSImpl(Scene& scene, std::function<void(Scene&, std::size_t)> op);
		};

		export class ENGINE_CORE_API Scene
		{
		public:
			Scene();
			~Scene();

			// Maybe we can implement scene copying later;
			// for now, no ownership issues please.
			Scene(const Scene& other) = delete;
			Scene& operator=(const Scene& other) = delete;

			Scene(Scene&& other);
			Scene& operator=(Scene&& other);

			Scene(std::string_view name, std::size_t expectedNodeCount = 1024);
			int AddNode(const Mat4x4& localTransform, int parent, const std::string& name);

			std::string_view GetName() const;
			int GetRootIndex() const;
			std::string_view GetNodeName(std::size_t index) const;

			void UpdateWorldTransforms();
			void WalkDepthFirst(std::size_t startingNode, std::function<void(Scene&, std::size_t)> op);
			void WalkBreadthFirst(std::size_t startingNode, std::function<void(Scene&, std::size_t)> op);

		private:
			// Not using a std::unique_ptr here to work around warning related to private std::unique_ptr
			// not having dll access
			SceneImpl* _impl { nullptr };
		};

		Scene::Scene() : Scene("NewScene", 1024)
		{
		}

		Scene::~Scene()
		{
			delete _impl;
		}

		Scene::Scene(Scene&& other)
		{
			if (_impl != other._impl)
			{
				_impl = other._impl;
				other._impl = nullptr;
			}
		}

		Scene& Scene::operator=(Scene&& other)
		{
			if (_impl != other._impl)
			{
				delete _impl;
				_impl = other._impl;
				other._impl = nullptr;
			}

			return *this;
		}

		Scene::Scene(std::string_view name, std::size_t expectedNodeCount)
			: _impl{ new SceneImpl(name, expectedNodeCount) }
		{
		}

		int Scene::GetRootIndex() const 
		{ 
			return 0; 
		}

		std::string_view Scene::GetNodeName(std::size_t nodeIndex) const 
		{ 
			assert(_impl != nullptr && "Invalid scene; possibly moved-from");
			return _impl->GetNodeName(nodeIndex); 
		}

		void Scene::WalkDepthFirst(std::size_t startingNode, std::function<void(Scene&, std::size_t)> op)
		{
			assert(_impl != nullptr && "Invalid scene; possibly moved-from");
			_impl->WalkDepthFirst(*this, startingNode, op);
		}

		void Scene::WalkBreadthFirst(std::size_t startingNode, std::function<void(Scene&, std::size_t)> op)
		{
			assert(_impl != nullptr && "Invalid scene; possibly moved-from");
			_impl->WalkBreadthFirst(*this, startingNode, op);
		}

		void Scene::UpdateWorldTransforms()
		{
			assert(_impl != nullptr && "Invalid scene; possibly moved-from");
			_impl->UpdateWorldTransforms(*this);
		}

		std::string_view Scene::GetName() const 
		{ 
			return GetNodeName(0); 
		}

		int Scene::AddNode(const Mat4x4& localTransform, int parent, const std::string& name)
		{
			assert(_impl != nullptr && "Invalid scene; possibly moved-from");
			return _impl->AddNode(localTransform, parent, name);
		}

		SceneImpl::SceneImpl(std::string_view name, std::size_t expectedNodeCount)
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

		void SceneImpl::UpdateWorldTransforms(Scene& scene)
		{
			WalkDepthFirst(scene, 0, [&](Scene& scene, std::size_t nodeIndex)
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

		int SceneImpl::AddNode(const Mat4x4& localTransform, int parent, const std::string& name)
		{
			assert(0 <= parent && parent < _nodes.size() && "Specified parent invalid; it should be a non-negative scene node index. (0 = scene root)");

			const int newNodeIndex = static_cast<int>(std::size(_nodes));

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

			_nodes.push_back(Node{});
			_hierarchy.push_back(Hierarchy{ parent, parentDepth + 1 });
			_globalTransforms.push_back(_globalTransforms[parent] * localTransform);
			_localTransforms.push_back(localTransform);
			_nodeNames.push_back(name);

			return newNodeIndex;
		};

		void SceneImpl::WalkDepthFirst(Scene& scene, std::size_t startingNode, std::function<void(Scene& scene, std::size_t currentNodeIndex)> op)
		{
			op(scene, startingNode);
			
			const auto node = _hierarchy[startingNode];
			if (node.firstChild != -1)
			{
				WalkDepthFirst(scene, node.firstChild, op);
			}

			int nextSibling = node.firstSibling;
			if (nextSibling != -1)
			{
				WalkDepthFirst(scene, nextSibling, op);
			}
		}

		void SceneImpl::WalkBreadthFirst(Scene& scene, std::size_t startingNode, std::function<void(Scene& scene, std::size_t currentNodeIndex)> op)
		{
			_walkHelperQueue.clear();
			_walkHelperQueue.push_back(static_cast<int>(startingNode));

			WalkBFSImpl(scene, op);
		}

		void SceneImpl::WalkBFSImpl(Scene& scene, std::function<void(Scene& scene, std::size_t currentNodeIndex)> op)
		{
			while (!_walkHelperQueue.empty())
			{
				const auto nextNode = _walkHelperQueue.front();
				_walkHelperQueue.pop_front();

				op(scene, nextNode);

				const auto& node = _hierarchy[nextNode];
				if (node.firstChild != -1)
				{
					_walkHelperQueue.push_back(node.firstChild);
				}

				auto nextSibling = node.firstSibling;
				while (nextSibling != -1)
				{
					op(scene, nextSibling);

					const auto siblingsFirstChild = _hierarchy[nextSibling].firstChild;
					if (siblingsFirstChild != -1)
					{
						_walkHelperQueue.push_back(siblingsFirstChild);
					}

					nextSibling = _hierarchy[nextSibling].firstSibling;
				}
			}
		}
	} // namespace Scene
} // namespace Engine