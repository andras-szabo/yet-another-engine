module;

#include <functional>
#include <string>

export module IScene;

import Component;
import Math;

namespace Engine
{
	export class IScene
	{
	public:
		virtual ~IScene() = default;

		virtual int AddNode(const Mat4x4& localTransform, 
			int parent, 
			const std::string& name, 
			Component* transformComponent = nullptr) = 0;

		virtual void SetLocalTransform(int nodeIndex,
			const Mat4x4& localTransform) = 0;

		virtual std::string_view GetName() const = 0;

		virtual void WalkDepthFirst(std::size_t startingNode,
			std::function<void(IScene&, std::size_t)> op) = 0;

		virtual void WalkBreadthFirst(std::size_t startingNode,
			std::function<void(IScene&, std::size_t)> op) = 0;

		virtual std::string_view GetNodeName(std::size_t) const = 0;
	};
}