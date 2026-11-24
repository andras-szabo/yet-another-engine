module;

#include <cassert>
#include "LoggerMacros.h"

module Scene;

#if defined ( __INTELLISENSE__ )
#include "Component.ixx"
#include "IComponentStorage.ixx"
#include "GameObject.ixx"
#include "GUID.ixx"
#include "Interfaces.ixx"
#include "Logger.ixx"
#include "Math.ixx"
#include "Scene.ixx"
#include "Transform.ixx"
#else
import Component;
import IComponentStorage;
import GameObject;
import GUID;
import Interfaces;
import Logger;
import Math;
import Transform;

import std;
#endif

namespace Engine
{
	namespace Scene
	{
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
				unsigned long long _guid = 0);

			void SetSceneName(std::string_view newName);

			void Clear();
		};

		void SceneImpl::Clear()
		{
			gameObjects.clear();
			nodes.clear();
			storage.Clear();
			walkHelperQueue.clear();
		}

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

		void Scene::Clear()
		{
			if (_impl != nullptr)
			{
				_impl->Clear();
			}
		}

		int Scene::GetNodeCount() const
		{
			return static_cast<int>(_impl->nodes.size());
		}

		Engine::GameObject* Scene::GetGameObject(std::size_t nodeIndex)
		{
			assert(nodeIndex < _impl->gameObjects.size() && "Node index out of bounds.");
			return _impl->gameObjects[nodeIndex].get();
		}

		const Engine::GameObject* Scene::GetGameObject(std::size_t nodeIndex) const
		{
			assert(nodeIndex < _impl->gameObjects.size() && "Node index out of bounds.");
			return _impl->gameObjects[nodeIndex].get();
		}

		const std::vector<std::unique_ptr<GameObject>>& Scene::GetAllGameObjects() const
		{
			return _impl->gameObjects;
		}

		SceneImpl::SceneImpl(IComponentStorage* componentStorage,
			std::string_view sceneName_,
			std::size_t expectedNodeCount,
			unsigned long long rootGuid)
			: storage{ expectedNodeCount }, sceneName{ sceneName_ }
		{
			nodes.reserve(expectedNodeCount);
			gameObjects.reserve(expectedNodeCount);

			const std::string rootName = "_Root";
			if (rootGuid == 0)
			{
				rootGuid = GUID().id;
			}

			CreateGameObject(componentStorage, rootName, -1, rootGuid);
		}

		void SceneImpl::SetSceneName(std::string_view newName)
		{
			sceneName = newName;
		}

		bool Scene::TryRemoveGameObject(std::size_t nodeIndex)
		{
			if (nodeIndex == 0)
			{
				LOG_ERROR("Cannot remove the root node of the scene.");
				return false;
			}

			if (nodeIndex >= _impl->gameObjects.size())
			{
				LOG_ERROR("Node index {} is out of bounds. Cannot remove GameObject.", nodeIndex);
				return false;
			}

			// NOTE: WalkBreadthFirst/WalkDepthFirst also visit the start node's *siblings*,
			// so they must not be used to collect a subtree. RemoveSubtree does it correctly.
			//
			// Removal is swap-and-pop: for every removed node, the last element takes its slot.
			// The storage tells us about each such move so we can mirror it in our own
			// parallel vectors. Assigning over gameObjects[dst] destroys the removed GameObject
			// (and thereby its components).
			const bool removed = _impl->storage.RemoveSubtree(static_cast<int>(nodeIndex),
				[this](int dst, int src)
				{
					if (dst != src)
					{
						_impl->gameObjects[dst] = std::move(_impl->gameObjects[src]);
						_impl->nodes[dst] = _impl->nodes[src];
					}
					_impl->gameObjects.pop_back();
					_impl->nodes.pop_back();
				});

			assert(_impl->storage.ValidateHierarchy() && "Hierarchy corrupted by RemoveSubtree");
			assert(_impl->gameObjects.size() == _impl->storage.hierarchies.size() &&
				_impl->nodes.size() == _impl->storage.hierarchies.size() &&
				"Scene vectors out of sync after removal");

			return removed;
		}

		GameObject* Scene::CreateGameObject(IComponentStorage* componentStorage,
			std::string_view name,
			int parentNodeIndex,
			unsigned long long _guid)
		{
			if (_guid == 0)
			{
				_guid = Engine::GUID().id;
			}

			return _impl->CreateGameObject(componentStorage, name, parentNodeIndex, _guid);
		}


		GameObject* SceneImpl::CreateGameObject(IComponentStorage* componentStorage,
			std::string_view name,
			int parentNodeIndex,
			unsigned long long _guid)
		{
			auto go = std::make_unique<GameObject>(name, _guid);
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
		{}

		const Engine::TransformStorage* Scene::GetTransformStorage() const
		{
			return &(*_impl).storage;
		}

		void Scene::UpdateWorldTransforms()
		{
			_impl->storage.UpdateWorldTransforms();
		}

		std::string_view Scene::GetSceneName() const
		{
			return _impl->sceneName;
		}

		void Scene::SetSceneName(std::string_view newName)
		{
			_impl->SetSceneName(newName);
		}

		Engine::Expected<void> Scene::SetNodeName(std::size_t nodeIndex, std::string_view newName)
		{
			assert(!_impl->storage.names.empty() && "Uninitialized scene!");

			if (newName.empty())
			{
				return Unexpected({ ErrorType::InvalidArgument, "Node name cannot be empty." });
			}

			if (nodeIndex < 0 || nodeIndex >= _impl->storage.names.size())
			{
				return Unexpected({ ErrorType::OutOfBounds, std::format("Node index {} is out of bounds.", nodeIndex) });
			}

			_impl->storage.names[nodeIndex] = newName;
			return {};
		}

		std::string_view Scene::GetNodeName(std::size_t nodeIndex) const
		{
			assert(!_impl->storage.names.empty() && "Uninitialized scene!");
			return _impl->storage.names[nodeIndex];
		}

		GUID Scene::GetNodeGuid(std::size_t nodeIndex) const
		{
			assert(nodeIndex < _impl->gameObjects.size() && "Node index out of bounds.");
			return _impl->gameObjects[nodeIndex]->GetGUID();
		}

		void Scene::WalkDepthFirst(std::size_t startingNode,
			std::function<void(Scene& scene, std::size_t currentNodeIndex, int depth)> op)
		{
			// Do NOT walk the starting node's siblings, only its subtree.
			WalkDFSImpl(startingNode, op, false);
		}

		void Scene::WalkDFSImpl(std::size_t startingNode,
			std::function<void(Scene& scene, std::size_t currentNodeIndex, int depth)> op,
			bool walkSiblings)
		{
			const auto node = _impl->storage.hierarchies[startingNode];
			op(*this, startingNode, node.depth);

			if (node.firstChild != -1)
			{
				WalkDFSImpl(node.firstChild, op, true);
			}

			if (walkSiblings)
			{
				int nextSibling = node.nextSibling;
				if (nextSibling != -1)
				{
					WalkDFSImpl(nextSibling, op, true);
				}
			}
		}

		void Scene::WalkBreadthFirst(std::size_t startingNode,
			std::function<void(Scene& scene, std::size_t currentNodeIndex, int depth)> op)
		{
			_impl->walkHelperQueue.clear();
			_impl->walkHelperQueue.push_back(static_cast<int>(startingNode));

			WalkBFSImpl(op);
		}

		void Scene::WalkBFSImpl(std::function<void(
			Scene& scene,
			std::size_t currentNodeIndex, int depth)> op)
		{
			bool isFirstIteration = true;
			while (!_impl->walkHelperQueue.empty())
			{
				const auto nextNode = _impl->walkHelperQueue.front();
				_impl->walkHelperQueue.pop_front();

				op(*this, nextNode, _impl->storage.hierarchies[nextNode].depth);

				const auto& node = _impl->storage.hierarchies[nextNode];
				if (node.firstChild != -1)
				{
					_impl->walkHelperQueue.push_back(node.firstChild);
				}

				if (!isFirstIteration)
				{
					auto nextSibling = node.nextSibling;
					while (nextSibling != -1)
					{
						op(*this, nextSibling, _impl->storage.hierarchies[nextSibling].depth);

						const auto siblingsFirstChild = _impl->storage.hierarchies[nextSibling].firstChild;
						if (siblingsFirstChild != -1)
						{
							_impl->walkHelperQueue.push_back(siblingsFirstChild);
						}

						nextSibling = _impl->storage.hierarchies[nextSibling].nextSibling;
					}
				}

				isFirstIteration = false;
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

		// TODO- consider cleanup. It's now not used.
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

#ifndef NDEBUG
			// The new parent must not be inside the subtree being moved (would create a cycle).
			for (int ancestor = newParentIndex; ancestor != -1; ancestor = _impl->storage.hierarchies[ancestor].parent)
			{
				assert(ancestor != nodeIndex && "Cannot reparent a node under one of its own descendants");
			}
#endif

			auto& hierarchies = _impl->storage.hierarchies;

			// O(1) unlink from the current parent's child/sibling chain
			_impl->storage.Unlink(nodeIndex);

			// Append as last child of the new parent
			int prevSibling = -1;
			if (hierarchies[newParentIndex].firstChild == -1)
			{
				hierarchies[newParentIndex].firstChild = nodeIndex;
			}
			else
			{
				prevSibling = hierarchies[newParentIndex].firstChild;
				while (hierarchies[prevSibling].nextSibling != -1)
				{
					prevSibling = hierarchies[prevSibling].nextSibling;
				}
				hierarchies[prevSibling].nextSibling = nodeIndex;
			}

			hierarchies[nodeIndex].parent = newParentIndex;
			hierarchies[nodeIndex].prevSibling = prevSibling;
			UpdateDepthsBelow(nodeIndex, hierarchies[newParentIndex].depth + 1);
			hierarchies[nodeIndex].isDirty = true;

			assert(_impl->storage.ValidateHierarchy() && "Hierarchy corrupted by SetParent");
		}

		void Scene::UpdateDepthsBelow(int nodeIndex, int newDepth)
		{
			_impl->storage.hierarchies[nodeIndex].depth = newDepth;
			int child = _impl->storage.hierarchies[nodeIndex].firstChild;
			while (child != -1)
			{
				UpdateDepthsBelow(child, newDepth + 1);
				child = _impl->storage.hierarchies[child].nextSibling;
			}
		}
	} // namespace Scene
} // namespace Engine
