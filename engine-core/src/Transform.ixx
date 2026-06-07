module;

#include <cassert>
#include <string>
#include <vector>

#include "engine_core_api.h"
#include "ReflectionMacros.h"

export module Transform;

import Component;
import ComponentRegistry;
import Interfaces;
import Math;
import Utility;

namespace Engine
{
	export struct Hierarchy
	{
		Hierarchy() = default;

		Hierarchy(int parent_, int depth_) : parent{ parent_ }, depth{ depth_ } {}
		Hierarchy(int parent_, int firstChild_, int firstSibling_) :
			parent{ parent_ },
			firstChild{ firstChild_ },
			firstSibling{ firstSibling_ }
		{
		}

		int parent{ -1 };
		int firstChild{ -1 };
		int firstSibling{ -1 };
		int depth{ -1 };
		bool isDirty{ false };
	};

	class Transform;

	export struct TransformStorage
	{
		TransformStorage(std::size_t expectedCount);

		int AddTransform(const Mat4x4& localTransform,
						 int parentIndex,
						 std::string_view name,
						 Transform* transform);

		void UpdateWorldTransforms();
		void WalkDepthFirst(std::size_t startingNode, std::function<void(std::size_t currentNodeIndex)> op);

		std::vector<Hierarchy> hierarchies;
		std::vector<Mat4x4> globalTransforms;
		std::vector<Mat4x4> localTransforms;
		std::vector<std::string> names;
		std::vector<Transform*> transformComponents;
	};

	export class ENGINE_CORE_API Transform : public Component, 
											 public ISceneNodeIndexObserver
	{
		COMPONENT_BODY(Transform)
		REGISTER_COMPONENT(Transform)

	public:
		// NOTE: Components must have default ctors :/
		Transform() = default;
		~Transform() override = default;

		Transform(TransformStorage* storage, 
			std::string_view name,
			int parentNodeIndex = 0);

		Vec3 GetLocalPosition() const;
		Quaternion GetLocalRotation() const;
		Vec3 GetLocalScale() const;

		void SetLocalPosition(Vec3 position);
		void SetLocalRotation(const Quaternion& rotation);
		void SetLocalScale(Vec3 scale);
		void SetLocalTRS(Vec3 position, const Quaternion& rotation, Vec3 scale);

		void OnSceneNodeIndexChanged(int newIndex) override;

		int GetSceneNodeIndex() const { return _sceneNodeIndex; }

	private:
		Mat4x4 CalculateLocalToWorldMatrix() const;
		void RefreshLocalToWorld();

		TransformStorage* _storage{ nullptr };
		int _sceneNodeIndex{ -1 };

		Vec3 _localPosition{ 0.0f, 0.0f, 0.0f };
		Quaternion _localRotation{ Quaternion::Identity() };
		Vec3 _localScale{ Vec3(1.0f, 1.0f, 1.0f) };
	};

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

		//hierarchies.emplace_back( -1, 0 );
		//globalTransforms.emplace_back(Mat4x4::Identity());
		//localTransforms.emplace_back(Mat4x4::Identity());
		//names.emplace_back(rootName);
		//transformComponents.emplace_back(nullptr);
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

		hierarchies.emplace_back(parentIndex, parent.depth);
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
}
