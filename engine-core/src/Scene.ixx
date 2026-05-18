module;

#include <cassert>
#include <functional>
#include <string>
#include <queue>
#include <type_traits>
#include <vector>

#include "engine_core_api.h"
#include "LoggerMacros.h"

export module Scene;

import Component;
import Interfaces;
import Logger;
import Math;
import SceneImpl;

namespace Engine
{
	namespace Scene
	{
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

			int AddNode(const Mat4x4& localTransform,
				int parent,
				const std::string& name,
				ISceneNodeIndexObserver* transformComponent = nullptr);

			void SetLocalTransform(int nodeIndex,
				const Mat4x4& localTransform);

			std::string_view GetName() const;
			std::string_view GetNodeName(std::size_t index) const;

			int GetRootIndex() const;
			
			const Mat4x4& GetLocalTransform(int nodeIndex) const;
			void UpdateNodeIndex(int oldIndex, int newIndex);
			void SetParent(int nodeIndex, int newParentIndex);

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
			_impl->WalkDepthFirst(startingNode, [&](std::size_t nodeIndex)
			{
					op(*this, nodeIndex);
			});
		}

		void Scene::WalkBreadthFirst(std::size_t startingNode, std::function<void(Scene&, std::size_t)> op)
		{
			assert(_impl != nullptr && "Invalid scene; possibly moved-from");
			_impl->WalkBreadthFirst(startingNode, [&](std::size_t nodeIndex)
			{
					op(*this, nodeIndex);
			});
		}

		void Scene::UpdateWorldTransforms()
		{
			assert(_impl != nullptr && "Invalid scene; possibly moved-from");
			_impl->UpdateWorldTransforms();
		}

		std::string_view Scene::GetName() const 
		{ 
			return GetNodeName(0); 
		}

		int Scene::AddNode(const Mat4x4& localTransform, int parent, const std::string& name, ISceneNodeIndexObserver* transformComponent)
		{
			assert(_impl != nullptr && "Invalid scene; possibly moved-from");
			return _impl->AddNode(localTransform, parent, name, transformComponent);
		}

		void Scene::SetLocalTransform(int nodeIndex, const Mat4x4& localTransform)
		{
			assert(_impl != nullptr && "Invalid scene; possibly moved-from");
			_impl->SetLocalTransform(nodeIndex, localTransform);
		}

		const Mat4x4& Scene::GetLocalTransform(int nodeIndex) const
		{
			assert(_impl != nullptr && "Invalid scene; possibly moved-from");
			return _impl->GetLocalTransform(nodeIndex);
		}

		void Scene::UpdateNodeIndex(int oldIndex, int newIndex)
		{
			assert(_impl != nullptr && "Invalid scene; possibly moved-from");
			_impl->UpdateNodeIndex(oldIndex, newIndex);
		}

		void Scene::SetParent(int nodeIndex, int newParentIndex)
		{
			assert(_impl != nullptr && "Invalid scene; possibly moved-from");
			_impl->SetParent(nodeIndex, newParentIndex);
		}

	
	} // namespace Scene
} // namespace Engine