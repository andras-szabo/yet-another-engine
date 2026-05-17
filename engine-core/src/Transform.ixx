module;

#include <cassert>
#include <string>

#include "engine_core_api.h"
#include "ReflectionMacros.h"

export module Transform;

import Component;
import EngineInstance;
import IScene;
import Math;
import Utility;

namespace Engine
{
	export class ENGINE_CORE_API Transform : public Component
	{
	public:
		Vec3 GetLocalPosition() const;
		Quaternion GetLocalRotation() const;
		Vec3 GetLocalScale() const;

		void SetLocalPosition(Vec3 position);
		void SetLocalRotation(const Quaternion& rotation);
		void SetLocalScale(Vec3 scale);
		void SetLocalTRS(Vec3 position, const Quaternion& rotation, Vec3 scale);

		void OnSceneNodeIndexChanged(int newIndex) override;

		void AddToScene(Engine::IScene& scene, const std::string& nodeName);

		int GetSceneNodeIndex() const { return _sceneNodeIndex; }

		COMPONENT_ID(Transform)

	private:
		Mat4x4 CreateLocalToWorld() const;
		void RefreshLocalToWorld();

		int _sceneNodeIndex{ -1 };

		Vec3 _localPosition;
		Quaternion _localRotation{ Quaternion::Identity() };
		Vec3 _localScale{ Vec3(1.0f, 1.0f, 1.0f) };
	};

	void Transform::AddToScene(Engine::IScene& scene, const std::string& nodeName)
	{
		assert(_sceneNodeIndex == -1 && "Trying to add transform to multiple scenes!");
		_sceneNodeIndex = scene.AddNode(CreateLocalToWorld(), 0, nodeName, this);
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

	Mat4x4 Transform::CreateLocalToWorld() const
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
		Instance.GetActiveScene().SetLocalTransform(_sceneNodeIndex, CreateLocalToWorld());
	}
}
