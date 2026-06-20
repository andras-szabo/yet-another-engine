module;

#include <cassert>
#include <cmath>
#include <format>

#include "engine_core_api.h"

export module Math;

import std;

namespace Engine
{
export constexpr float PI = 3.14159265f;
export constexpr float TO_RAD = PI / 180.0f;
export constexpr float TO_DEG = 180.0f / PI;

export struct ENGINE_CORE_API Vec2
{
	float x{ 0.f };
	float y{ 0.f };

	constexpr Vec2() = default;
	constexpr Vec2(float x_, float y_) : x{ x_ }, y{ y_ } {}

	constexpr float operator[](int index) const;
	constexpr float& operator[](int index);

	constexpr Vec2 operator+(const Vec2 other) const;
	constexpr Vec2 operator-(const Vec2 other) const;
	constexpr Vec2 operator*(float scalar) const;
	constexpr Vec2 operator/(float scalar) const;

	constexpr Vec2 operator-() const;

	constexpr Vec2& operator+=(const Vec2 other);
	constexpr Vec2& operator-=(const Vec2 other);
	constexpr Vec2& operator*=(float scalar);
	constexpr Vec2& operator/=(float scalar);

	constexpr float Dot(const Vec2 other) const;
	constexpr float SqrMagnitude() const;

	float Length() const;

	void Normalize();
	void NormalizeSafe(const Vec2 fallback = Vec2::Up());

	Vec2 Normalized() const;
	Vec2 NormalizedSafe(const Vec2 fallback = Vec2::Up()) const;

	static constexpr Vec2 Up();
	static constexpr Vec2 Right();
	static constexpr Vec2 Down();
	static constexpr Vec2 Left();
};

export struct ENGINE_CORE_API Vec3
{
	float x{ 0.f };
	float y{ 0.f };
	float z{ 0.f };

	constexpr Vec3() = default;
	constexpr Vec3(float x_, float y_, float z_) : x{ x_ }, y{ y_ }, z{ z_ } {}
	constexpr explicit Vec3(const Vec2 other);
	constexpr Vec3(const Vec2, float);

	constexpr Vec2 xy() const;
	constexpr Vec2 xz() const;
	constexpr Vec2 yz() const;

	constexpr float operator[](int index) const;
	constexpr float& operator[](int index);

	constexpr Vec3 operator+(const Vec3 other) const;
	constexpr Vec3 operator-(const Vec3 other) const;
	constexpr Vec3 operator*(float scalar) const;
	constexpr Vec3 operator/(float scalar) const;

	constexpr Vec3 operator-() const;

	constexpr Vec3& operator+=(const Vec3 other);
	constexpr Vec3& operator-=(const Vec3 other);
	constexpr Vec3& operator*=(float scalar);
	constexpr Vec3& operator/=(float scalar);

	constexpr float Dot(const Vec3 other) const;
	constexpr Vec3 Cross(const Vec3 other) const;
	constexpr float SqrMagnitude() const;

	float Length() const;

	void Normalize();
	void NormalizeSafe(const Vec3 fallback = Vec3::Up());

	Vec3 Normalized() const;
	Vec3 NormalizedSafe(const Vec3 fallback = Vec3::Up()) const;

	static constexpr Vec3 Up();
	static constexpr Vec3 Right();
	static constexpr Vec3 Down();
	static constexpr Vec3 Left();
	static constexpr Vec3 Forward();
	static constexpr Vec3 Back();
};

export struct ENGINE_CORE_API Vec4
{
	float x{ 0.f };
	float y{ 0.f };
	float z{ 0.f };
	float w{ 0.f };

	constexpr Vec4() = default;
	constexpr Vec4(float x_, float y_, float z_, float w_) : x{ x_ }, y{ y_ }, z{ z_ }, w{ w_ } {}
	constexpr Vec4(const Vec3, float);
	constexpr Vec4(const Vec2, const Vec2);

	constexpr Vec3 xyz() const;
	constexpr Vec2 xy() const;
	constexpr Vec2 zw() const;

	constexpr float operator[](int index) const;
	constexpr float& operator[](int index);

	constexpr Vec4 operator+(const Vec4 other) const;
	constexpr Vec4 operator-(const Vec4 other) const;
	constexpr Vec4 operator*(float scalar) const;
	constexpr Vec4 operator/(float scalar) const;

	constexpr Vec4 operator-() const;

	constexpr Vec4& operator+=(const Vec4 other);
	constexpr Vec4& operator-=(const Vec4 other);
	constexpr Vec4& operator*=(float scalar);
	constexpr Vec4& operator/=(float scalar);

	constexpr float Dot(const Vec4 other) const;
	constexpr float SqrMagnitude() const;

	float Length() const;

	void Normalize();
	void NormalizeSafe(const Vec4 fallback = Vec4(1.f, 0.f, 0.f, 0.f));

	Vec4 Normalized() const;
	Vec4 NormalizedSafe(const Vec4 fallback = Vec4(1.f, 0.f, 0.f, 0.f)) const;
};

export struct ENGINE_CORE_API Mat3x3
{
	float m[9]{ 0.f, 0.f, 0.f,
				0.f, 0.f, 0.f,
				0.f, 0.f, 0.f };

	constexpr Mat3x3() = default;
	constexpr Mat3x3(float m0, float m1, float m2,
		float m3, float m4, float m5,
		float m6, float m7, float m8);

	constexpr Mat3x3(const Vec3 r0, const Vec3 r1, const Vec3 r2);

	constexpr Vec3 Row(int index) const;
	constexpr Vec3 Col(int index) const;

	constexpr float At(int row, int col) const;
	constexpr float& At(int row, int col);

	constexpr Mat3x3 operator*(const Mat3x3& other) const;
	constexpr Mat3x3 Transposed() const;

	static constexpr Mat3x3 Identity();
};

struct Quaternion;

export struct ENGINE_CORE_API Mat4x4
{
	float m[16]{ 0.f, 0.f, 0.f, 0.f,
				  0.f, 0.f, 0.f, 0.f,
				  0.f, 0.f, 0.f, 0.f,
				  0.f, 0.f, 0.f, 0.f };

	constexpr Mat4x4() = default;

	constexpr Mat4x4 operator*(const Mat4x4& other) const;
	constexpr Mat4x4 Transposed() const;
	constexpr Mat4x4 Inverse() const;
	constexpr float Determinant() const;

	constexpr void SetRow(int index, float x, float y, float z, float w);

	static constexpr Mat4x4 Identity();
	static constexpr Mat4x4 Scale(Vec3 scale);
	static constexpr Mat4x4 Scale(float x, float y, float z);
	static constexpr Mat4x4 Translate(Vec3 position);
	static constexpr Mat4x4 TRS(Vec3 translation, const Quaternion& rotation, Vec3 scale);
	static constexpr Mat4x4 FromQuaternion(float w, float x, float y, float z);
};

export enum class ENGINE_CORE_API RotateOrder
{
	RollPitchYaw,
	RollYawPitch,
	PitchRollYaw,
	PitchYawRoll,
	YawRollPitch,
	YawPitchRoll
};

export struct ENGINE_CORE_API Quaternion
{
	float x{ 0.0f };
	float y{ 0.0f };
	float z{ 0.0f };
	float w{ 0.0f };

	constexpr Quaternion() = default;
	constexpr Quaternion(float w_, float x_, float y_, float z_) : x{ x_ }, y{ y_ }, z{ z_ }, w{ w_ } {}
	constexpr Quaternion(float w_, const Vec3 v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ w_ } {}

	explicit Quaternion(const Mat3x3& matrix);

	constexpr Quaternion operator*(const Quaternion& other) const;
	constexpr Quaternion operator-(const Quaternion& other) const;
	constexpr Quaternion operator*(float scalar) const;
	
	constexpr Quaternion& operator/=(float scalar);
	constexpr Quaternion& operator+=(const Quaternion& other);

	constexpr Quaternion Inverse() const;
	float Magnitude() const;

	constexpr static Quaternion Identity();
	static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t, bool forceLerp);
	static Quaternion AngleAxis(float angleDegrees, const Vec3 axis);
	static Quaternion LookRotation(const Vec3 targetDirection, const Vec3 targetRight, const Vec3 targetUp);
	static Quaternion Euler(const Vec3 degrees, RotateOrder order = RotateOrder::RollPitchYaw);
	static Quaternion Euler(float pitch, float yaw, float roll, RotateOrder order = RotateOrder::RollPitchYaw);
	constexpr static Quaternion FromTo(const Quaternion& from, const Quaternion& to);
};

export constexpr ENGINE_CORE_API Vec3 operator*(const Mat3x3& m, const Vec3 v);
export constexpr ENGINE_CORE_API Vec4 operator*(const Mat4x4& m, const Vec4 v);

// t is unclamped; values outside [0, 1] will extrapolate
export constexpr ENGINE_CORE_API Vec2 Lerp(const Vec2 a, const Vec2 b, float t) noexcept;
export constexpr ENGINE_CORE_API Vec2 operator*(float scalar, const Vec2 v) noexcept;

// t is not clamped; values outside [0, 1] will extrapolate
export constexpr ENGINE_CORE_API Vec3 Lerp(const Vec3 a, const Vec3 b, float t) noexcept;
export constexpr ENGINE_CORE_API Vec3 operator*(float scalar, const Vec3 v) noexcept;

// t is not clamped; values outside [0, 1] will extrapolate
export constexpr ENGINE_CORE_API Vec4 Lerp(const Vec4 a, const Vec4 b, float t) noexcept;
export constexpr ENGINE_CORE_API Vec4 operator*(float scalar, const Vec4 v) noexcept; 

} // namespace Engine

template<>
struct std::formatter<Engine::Vec4>
{
private:
	mutable std::formatter<float> _floatFormatter;

public:
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return _floatFormatter.parse(ctx);
	}

	auto format(Engine::Vec4 vec, std::format_context& ctx) const
	{
		auto out = ctx.out();
		*out++ = '(';
		out = _floatFormatter.format(vec.x, ctx);
		*out++ = ';';
		*out++ = ' ';
		out = _floatFormatter.format(vec.y, ctx);
		*out++ = ';';
		*out++ = ' ';
		out = _floatFormatter.format(vec.z, ctx);
		*out++ = ';';
		*out++ = ' ';
		out = _floatFormatter.format(vec.w, ctx);
		*out++ = ')';
		return out;
	}
};

template<>
struct std::formatter<Engine::Vec3>
{
private:
	mutable std::formatter<float> _floatFormatter;

public:
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return _floatFormatter.parse(ctx);
	}

	auto format(Engine::Vec3 vec, std::format_context& ctx) const
	{
		auto out = ctx.out();
		*out++ = '(';
		out = _floatFormatter.format(vec.x, ctx);
		*out++ = ';';
		*out++ = ' ';
		out = _floatFormatter.format(vec.y, ctx);
		*out++ = ';';
		*out++ = ' ';
		out = _floatFormatter.format(vec.z, ctx);
		*out++ = ')';
		return out;
	}
};


template<>
struct std::formatter<Engine::Vec2>
{
private:
	mutable std::formatter<float> _floatFormatter;

public:
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return _floatFormatter.parse(ctx);
	}

	auto format(Engine::Vec2 vec, std::format_context& ctx) const
	{
		auto out = ctx.out();
		*out++ = '(';
		out = _floatFormatter.format(vec.x, ctx);
		*out++ = ';';
		*out++ = ' ';
		out = _floatFormatter.format(vec.y, ctx);
		*out++ = ')';
		return out;
	}
};
