module;

#include "engine_core_api.h"

export module Transform;

import Math;

export 
class ENGINE_CORE_API Transform
{
public:
	constexpr Vec3 GetLocalPosition() const;
	constexpr Quaternion GetLocalRotation() const;
	constexpr Vec3 GetLocalScale() const;

	constexpr void SetLocalPosition(Vec3 position);
	constexpr void SetLocalRotation(const Quaternion& rotation);
	constexpr void SetLocalScale(Vec3 scale);
	constexpr void SetLocalTRS(Vec3 position, const Quaternion& rotation, Vec3 scale);

private:
	constexpr void RefreshLocalToWorld();

	Mat4x4 _localToWorld { Mat4x4::Identity() };

	Vec3 _localPosition;
	Quaternion _localRotation { Quaternion::Identity() };
	Vec3 _localScale { Vec3(1.0f, 1.0f, 1.0f) };
};

constexpr Vec3 Transform::GetLocalPosition() const
{
	const auto& m = _localToWorld.m;
	return Vec3(m[3], m[7], m[11]);
}

constexpr Quaternion Transform::GetLocalRotation() const
{
	return _localRotation;
}

constexpr Vec3 Transform::GetLocalScale() const
{
	return _localScale;
}

constexpr void Transform::SetLocalTRS(Vec3 position, const Quaternion& rotation, Vec3 scale)
{
	_localPosition = position;
	_localRotation = rotation;
	_localScale = scale;

	RefreshLocalToWorld();
}

constexpr void Transform::SetLocalPosition(Vec3 position)
{
	_localPosition = position;

	auto& m = _localToWorld.m;

	m[3] = position.x;
	m[7] = position.y;
	m[11] = position.z;
}

constexpr void Transform::SetLocalRotation(const Quaternion& rotation)
{
	_localRotation = rotation;
	RefreshLocalToWorld();
}

constexpr void Transform::SetLocalScale(Vec3 scale)
{
	_localScale = scale;
	RefreshLocalToWorld();
}

constexpr void Transform::RefreshLocalToWorld()
{
	const auto scale = Mat4x4::Scale(_localScale);
	const auto rotation = Mat4x4::FromQuaternion(_localRotation.w, 
		_localRotation.x, 
		_localRotation.y, 
		_localRotation.z);

	_localToWorld = rotation * scale;
	auto& m = _localToWorld.m;

	m[3] = _localPosition.x;
	m[7] = _localPosition.y;
	m[11] = _localPosition.z;
	m[15] = 1.0f;
}
