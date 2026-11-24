module;

#include "engine_core_api.h"
#include "ReflectionMacros.h"

export module Transform;

#if defined ( __INTELLISENSE__ )
#include "Component.ixx"
#include "ComponentRegistry.ixx"
#include "Interfaces.ixx"
#include "Math.ixx"
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
	export struct Hierarchy
	{
		Hierarchy() = default;

		Hierarchy(int parent_, int depth_) : parent{ parent_ }, depth{ depth_ } {}
		Hierarchy(int parent_, int firstChild_, int nextSibling_) :
			parent{ parent_ },
			firstChild{ firstChild_ },
			nextSibling{ nextSibling_ }
		{
		}

		int parent{ -1 };		// Direct parent
		int firstChild{ -1 };
		int nextSibling{ -1 };	// Immediate right sibling
		int prevSibling{ -1 };	// Immediate left sibling (-1 if this is the first child)
		int depth{ -1 };
		bool isDirty{ false };
	};

	class Transform;

	export struct TransformStorage
	{
		ENGINE_CORE_API TransformStorage(std::size_t expectedCount);

		ENGINE_CORE_API int AddTransform(const Mat4x4& localTransform,
			int parentIndex,
			std::string_view name,
			Transform* transform);

		ENGINE_CORE_API void UpdateWorldTransforms();
		ENGINE_CORE_API void WalkDepthFirst(std::size_t startingNode, std::function<void(std::size_t currentNodeIndex)> op);
		ENGINE_CORE_API void Clear();

		// Detaches nodeIndex from its parent and sibling chain in O(1). Does not touch depth.
		ENGINE_CORE_API void Unlink(int nodeIndex);

		// Removes a node and its whole subtree. The scene root (0) cannot be removed.
		// Freed slots are filled by moving the last node into them (swap-and-pop), so node
		// indices are NOT stable, and parent index < child index is not preserved.
		// onSlotReused(dst, src) is invoked once per removed node, before the storage's own
		// pop_back: the owner should move its parallel element src -> dst (overwriting the
		// removed one; if dst == src, nothing to move) and then pop_back its own vector.
		// Returns false (and changes nothing) if nodeIndex is invalid or the root.
		ENGINE_CORE_API bool RemoveSubtree(int nodeIndex,
			const std::function<void(int dst, int src)>& onSlotReused = {});

		// Checks that all links (parent / firstChild / nextSibling / prevSibling / depth)
		// are mutually consistent and that the parallel vectors are in sync.
		// Returns true if valid; otherwise false and, if outError != nullptr, a description.
		ENGINE_CORE_API bool ValidateHierarchy(std::string* outError = nullptr) const;

		std::vector<Hierarchy> hierarchies;
		std::vector<Mat4x4> globalTransforms;
		std::vector<Mat4x4> localTransforms;
		std::vector<std::string> names;
		std::vector<Transform*> transformComponents;

	private:
		void MoveNode(int dst, int src);
	};

	export class ENGINE_CORE_API Transform : public Component,
		public ISceneNodeIndexObserver
	{
		COMPONENT_BODY(Transform)

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
		void OnCreate() override;

		int GetSceneNodeIndex() const { return _sceneNodeIndex; }
		std::string_view GetName() const { return _storage->names[_sceneNodeIndex]; }

	private:
		Mat4x4 CalculateLocalToWorldMatrix() const;
		void RefreshLocalToWorld();

		TransformStorage* _storage{ nullptr };
		int _sceneNodeIndex{ -1 };

		Vec3 _localPosition{ 0.0f, 0.0f, 0.0f };
		Quaternion _localRotation{ Quaternion::Identity() };
		Vec3 _localScale{ Vec3(1.0f, 1.0f, 1.0f) };
	};

	//Engine::FieldSpan Transform::GetFieldDescriptors()
	//{
	//	static const Engine::FieldDescriptor fields[]
	//	{
	//		{ "_localPosition", Engine::FieldType::Vec3, offsetof(Transform, _localPosition) }
	//	};

	//	return Engine::FieldSpan{ &fields[0], sizeof(fields) / sizeof(fields[0]) };
	//}
    //{                                                                               \
    //    static const Engine::FieldDescriptor fields[] { __VA_ARGS__ };              \
    //    return Engine::FieldSpan { &fields, sizeof(fields) / sizeof(fields[0])};    \
    //}      

} // namespace Engine

