module;

#include <cassert>
#include "ReflectionMacros.h"

module Transform;

#if defined ( __INTELLISENSE__ )
#include "Component.ixx"
#include "ComponentRegistry.ixx"
#include "Interfaces.ixx"
#include "Math.ixx"
#include "Transform.ixx"
#include "Utility.ixx"
#else
import Component;
import ComponentRegistry;
import Interfaces;
import Math;
import Utility;
import std;
#endif

namespace Engine
{
	REGISTER_COMPONENT(Transform)
	REFLECTED_FIELDS(Transform,
			{ "_localPosition", Engine::FieldType::Vec3, offsetof(Transform, _localPosition) },
			{ "_localRotation", Engine::FieldType::Quaternion, offsetof(Transform, _localRotation) },
			{ "_localScale", Engine::FieldType::Vec3, offsetof(Transform, _localScale) })

	TransformStorage::TransformStorage(std::size_t expectedNodeCount)
	{
		hierarchies.reserve(expectedNodeCount);
		globalTransforms.reserve(expectedNodeCount);
		localTransforms.reserve(expectedNodeCount);
		names.reserve(expectedNodeCount);
		transformComponents.reserve(expectedNodeCount);
	}

	void TransformStorage::Clear()
	{
		hierarchies.clear();
		globalTransforms.clear();
		localTransforms.clear();
		names.clear();
		transformComponents.clear();
	}

	int TransformStorage::AddTransform(const Mat4x4& localTransform,
		int parentIndex,
		std::string_view name,
		Transform* transform)
	{
		if (parentIndex == -1)		// We're dealing with a scene root
		{
			assert(hierarchies.empty() && "Scene can only have one root!");

			hierarchies.emplace_back(-1, 0);
			globalTransforms.emplace_back(localTransform);
			localTransforms.emplace_back(localTransform);
			names.emplace_back(name);
			transformComponents.emplace_back(transform);

			return 0;
		}

		assert(0 <= parentIndex && parentIndex < (int)hierarchies.size() &&
			"Specified parent invalid; it should be a non-negative scene node index. (0 = scene root)");

		const int newNodeIndex = static_cast<int>(std::size(hierarchies));

		assert(newNodeIndex == (int)hierarchies.size() &&
			newNodeIndex == (int)globalTransforms.size() &&
			newNodeIndex == (int)localTransforms.size() &&
			newNodeIndex == (int)names.size() &&
			newNodeIndex == (int)transformComponents.size() &&
			"Scene graph vectors out of sync!");

		int prevSibling = -1;
		if (hierarchies[parentIndex].firstChild == -1)
		{
			hierarchies[parentIndex].firstChild = newNodeIndex;
		}
		else
		{
			prevSibling = hierarchies[parentIndex].firstChild;
			while (hierarchies[prevSibling].nextSibling != -1)
			{
				prevSibling = hierarchies[prevSibling].nextSibling;
			}
			hierarchies[prevSibling].nextSibling = newNodeIndex;
		}

		// (Must not hold a reference into 'hierarchies' across emplace_back.)
		const int newDepth = hierarchies[parentIndex].depth + 1;
		hierarchies.emplace_back(parentIndex, newDepth);
		hierarchies.back().prevSibling = prevSibling;
		globalTransforms.emplace_back(globalTransforms[parentIndex] * localTransform);
		localTransforms.emplace_back(localTransform);
		names.emplace_back(name);
		transformComponents.emplace_back(transform);

		return newNodeIndex;
	}

	void TransformStorage::UpdateWorldTransforms()
	{
		WalkDepthFirst(0, [&](std::size_t nodeIndex)
			{
				if (hierarchies[nodeIndex].isDirty)
				{
					const auto parent = hierarchies[nodeIndex].parent;
					if (parent == -1)
					{
						globalTransforms[nodeIndex] = localTransforms[nodeIndex];
					}
					else
					{
						globalTransforms[nodeIndex] = globalTransforms[parent] * localTransforms[nodeIndex];
					}
					hierarchies[nodeIndex].isDirty = false;
				}
			});
	}

	void TransformStorage::WalkDepthFirst(std::size_t startingNode,
		std::function<void(std::size_t currentNodeIndex)> op)
	{
		op(startingNode);

		const auto node = hierarchies[startingNode];
		if (node.firstChild != -1)
		{
			WalkDepthFirst(node.firstChild, op);
		}

		int nextSibling = node.nextSibling;
		if (nextSibling != -1)
		{
			WalkDepthFirst(nextSibling, op);
		}
	}

	void TransformStorage::Unlink(int nodeIndex)
	{
		assert(0 <= nodeIndex && nodeIndex < (int)hierarchies.size());

		Hierarchy& h = hierarchies[nodeIndex];

		if (h.prevSibling != -1)
		{
			hierarchies[h.prevSibling].nextSibling = h.nextSibling;
		}
		else if (h.parent != -1)
		{
			hierarchies[h.parent].firstChild = h.nextSibling;
		}

		if (h.nextSibling != -1)
		{
			hierarchies[h.nextSibling].prevSibling = h.prevSibling;
		}

		h.prevSibling = -1;
		h.nextSibling = -1;
	}

	void TransformStorage::MoveNode(int dst, int src)
	{
		hierarchies[dst] = hierarchies[src];
		globalTransforms[dst] = globalTransforms[src];
		localTransforms[dst] = localTransforms[src];
		names[dst] = std::move(names[src]);
		transformComponents[dst] = transformComponents[src];

		// Repair everything that referred to 'src' so that it refers to 'dst'.
		const Hierarchy& h = hierarchies[dst];
		if (h.prevSibling != -1)
		{
			hierarchies[h.prevSibling].nextSibling = dst;
		}
		else if (h.parent != -1)
		{
			hierarchies[h.parent].firstChild = dst;
		}

		if (h.nextSibling != -1)
		{
			hierarchies[h.nextSibling].prevSibling = dst;
		}

		for (int child = h.firstChild; child != -1; child = hierarchies[child].nextSibling)
		{
			hierarchies[child].parent = dst;
		}

		if (transformComponents[dst] != nullptr)
		{
			transformComponents[dst]->OnSceneNodeIndexChanged(dst);
		}
	}

	bool TransformStorage::RemoveSubtree(int nodeIndex,
		const std::function<void(int dst, int src)>& onSlotReused)
	{
		if (nodeIndex <= 0 || nodeIndex >= static_cast<int>(hierarchies.size()))
		{
			return false;
		}

		// Detach the subtree root first; after this, nothing outside the subtree
		// refers to anything inside it, and the root's siblings are out of reach.
		Unlink(nodeIndex);

		std::vector<int> doomed;
		std::vector<int> pending{ nodeIndex };

		while (!pending.empty())
		{
			const int current = pending.back();
			pending.pop_back();
			doomed.push_back(current);

			for (int child = hierarchies[current].firstChild; 
				child != -1; 
				child = hierarchies[child].nextSibling)
			{
				pending.push_back(child);
			}
		}

		// Highest index first: this guarantees that the last slot is always a survivor,
		// since every doomed index still to be processed is lower than it.
		std::ranges::sort(doomed, std::greater<>{});

		for (const int idx : doomed)
		{
			const int last = static_cast<int>(hierarchies.size()) - 1;
			if (idx != last)
			{
				MoveNode(idx, last);
			}

			if (onSlotReused)
			{
				onSlotReused(idx, last);
			}

			hierarchies.pop_back();
			globalTransforms.pop_back();
			localTransforms.pop_back();
			names.pop_back();
			transformComponents.pop_back();
		}

		return true;
	}

	bool TransformStorage::ValidateHierarchy(std::string* outError) const
	{
		auto fail = [&](std::string message)
			{
				if (outError != nullptr)
				{
					*outError = std::move(message);
				}
				return false;
			};

		const int count = static_cast<int>(hierarchies.size());
		if (count != (int)globalTransforms.size() ||
			count != (int)localTransforms.size() ||
			count != (int)names.size() ||
			count != (int)transformComponents.size())
		{
			return fail("Parallel vectors out of sync.");
		}

		if (count == 0)
		{
			return true;
		}

		auto inRange = [&](int i) { return i == -1 || (0 <= i && i < count); };

		const Hierarchy& root = hierarchies[0];
		if (root.parent != -1 || root.prevSibling != -1 || root.nextSibling != -1 || root.depth != 0)
		{
			return fail("Root (0) must have no parent/siblings and depth 0.");
		}

		int reachable = 0;
		for (int i = 0; i < count; ++i)
		{
			const Hierarchy& h = hierarchies[i];
			if (!inRange(h.parent) || !inRange(h.firstChild) || !inRange(h.nextSibling) || !inRange(h.prevSibling))
			{
				return fail(std::format("Node {}: link index out of range.", i));
			}

			if (i != 0 && h.parent == -1)
			{
				return fail(std::format("Node {}: non-root node without a parent.", i));
			}

			if (h.nextSibling != -1)
			{
				if (hierarchies[h.nextSibling].prevSibling != i)
				{
					return fail(std::format("Node {}: nextSibling {} does not point back via prevSibling.", i, h.nextSibling));
				}
				if (hierarchies[h.nextSibling].parent != h.parent)
				{
					return fail(std::format("Node {}: sibling {} has a different parent.", i, h.nextSibling));
				}
			}

			if (h.prevSibling != -1)
			{
				if (hierarchies[h.prevSibling].nextSibling != i)
				{
					return fail(std::format("Node {}: prevSibling {} does not point back via nextSibling.", i, h.prevSibling));
				}
			}
			else if (h.parent != -1 && hierarchies[h.parent].firstChild != i)
			{
				return fail(std::format("Node {}: has no prevSibling but is not firstChild of parent {}.", i, h.parent));
			}

			if (h.firstChild != -1)
			{
				const Hierarchy& child = hierarchies[h.firstChild];
				if (child.parent != i || child.prevSibling != -1)
				{
					return fail(std::format("Node {}: firstChild {} has wrong parent or a prevSibling.", i, h.firstChild));
				}
			}

			if (h.parent != -1 && h.depth != hierarchies[h.parent].depth + 1)
			{
				return fail(std::format("Node {}: depth {} is not parent's depth + 1.", i, h.depth));
			}

			if (transformComponents[i] != nullptr && transformComponents[i]->GetSceneNodeIndex() != i)
			{
				return fail(std::format("Node {}: Transform component has stale scene node index {}.",
					i, transformComponents[i]->GetSceneNodeIndex()));
			}
		}

		// Everything must be reachable from the root exactly once (no cycles, no orphans).
		// Iterative preorder walk; bounded by 'count' steps to guard against cycles.
		int node = 0;
		while (node != -1)
		{
			if (++reachable > count)
			{
				return fail("Cycle detected in hierarchy.");
			}

			if (hierarchies[node].firstChild != -1)
			{
				node = hierarchies[node].firstChild;
			}
			else
			{
				while (node != -1 && hierarchies[node].nextSibling == -1)
				{
					node = hierarchies[node].parent;
				}
				if (node != -1)
				{
					node = hierarchies[node].nextSibling;
				}
			}
		}

		if (reachable != count)
		{
			return fail(std::format("{} node(s) unreachable from the root.", count - reachable));
		}

		return true;
	}

	Transform::Transform(TransformStorage* storage,
		std::string_view name,
		int parentNodeIndex)
		: _storage{ storage }
	{
		assert(storage != nullptr && "Trying to create transform with null storage");

		_sceneNodeIndex = storage->AddTransform(CalculateLocalToWorldMatrix(),
			parentNodeIndex,
			name,
			this);
	}

	void Transform::OnSceneNodeIndexChanged(int newIndex)
	{
		_sceneNodeIndex = newIndex;
	}

	Vec3 Transform::GetLocalPosition() const
	{
		return _localPosition;
	}

	Quaternion Transform::GetLocalRotation() const
	{
		return _localRotation;
	}

	Vec3 Transform::GetLocalScale() const
	{
		return _localScale;
	}

	void Transform::SetLocalTRS(Vec3 position, const Quaternion& rotation, Vec3 scale)
	{
		_localPosition = position;
		_localRotation = rotation;
		_localScale = scale;

		RefreshLocalToWorld();
	}

	void Transform::SetLocalPosition(Vec3 position)
	{
		_localPosition = position;
		RefreshLocalToWorld();
	}

	void Transform::SetLocalRotation(const Quaternion& rotation)
	{
		_localRotation = rotation;
		RefreshLocalToWorld();
	}

	void Transform::SetLocalScale(Vec3 scale)
	{
		_localScale = scale;
		RefreshLocalToWorld();
	}

	Mat4x4 Transform::CalculateLocalToWorldMatrix() const
	{
		const auto scale = Mat4x4::Scale(_localScale);
		const auto rotation = Mat4x4::FromQuaternion(_localRotation.w,
			_localRotation.x,
			_localRotation.y,
			_localRotation.z);

		Mat4x4 localToWorld = rotation * scale;
		auto& m = localToWorld.m;

		m[3] = _localPosition.x;
		m[7] = _localPosition.y;
		m[11] = _localPosition.z;
		m[15] = 1.0f;

		return localToWorld;
	}

	void Transform::RefreshLocalToWorld()
	{
		_storage->localTransforms[_sceneNodeIndex] = CalculateLocalToWorldMatrix();
		_storage->hierarchies[_sceneNodeIndex].isDirty = true;
	}

	void Transform::OnCreate()
	{
		RefreshLocalToWorld();
	}
} // namespace Engine