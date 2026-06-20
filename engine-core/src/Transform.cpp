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

		auto& parent = hierarchies[parentIndex];
		if (parent.firstChild == -1)
		{
			parent.firstChild = newNodeIndex;
		}
		else
		{
			int nextNodeToCheck = parent.firstChild;
			while (hierarchies[nextNodeToCheck].firstSibling != -1)
			{
				nextNodeToCheck = hierarchies[nextNodeToCheck].firstSibling;
			}
			hierarchies[nextNodeToCheck].firstSibling = newNodeIndex;
		}

		hierarchies.emplace_back(parentIndex, parent.depth + 1);
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

		int nextSibling = node.firstSibling;
		if (nextSibling != -1)
		{
			WalkDepthFirst(nextSibling, op);
		}
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