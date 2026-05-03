module;

#include <cassert>
#include <cmath>
#include <format>

#include "engine_core_api.h"

export module Math;

export const float ENGINE_CORE_API PI = 3.14159265f;
export const float ENGINE_CORE_API TO_RAD = PI / 180.0f;
export const float ENGINE_CORE_API TO_DEG = 180.0f / PI;

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
	float m[9]{ 1.f, 0.f, 0.f,
				0.f, 1.f, 0.f,
				0.f, 0.f, 1.f };

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
	float m[16]{ 1.f, 0.f, 0.f, 0.f,
				  0.f, 1.f, 0.f, 0.f,
				  0.f, 0.f, 1.f, 0.f,
				  0.f, 0.f, 0.f, 1.f };

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
	float w{ 1.0f };

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

	static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t, bool forceLerp);
	static Quaternion AngleAxis(float angleDegrees, const Vec3 axis);
	static Quaternion LookRotation(const Vec3 targetDirection, const Vec3 targetRight, const Vec3 targetUp);
	static Quaternion Euler(const Vec3 degrees, RotateOrder order = RotateOrder::RollPitchYaw);
	static Quaternion Euler(float pitch, float yaw, float roll, RotateOrder order = RotateOrder::RollPitchYaw);
	constexpr static Quaternion FromTo(const Quaternion& from, const Quaternion& to);
};

constexpr Mat4x4 Mat4x4::operator*(const Mat4x4& other) const
{
	Mat4x4 result;

	for (int row = 0; row < 4; ++row)
	{
		for (int col = 0; col < 4; ++col)
		{
			for (int k = 0; k < 4; ++k)
			{
				result.m[row * 4 + col] += m[row * 4 + k] * other.m[k * 4 + col];
			}
		}
	}

	return result;
}

constexpr Mat4x4 Mat4x4::Transposed() const
{
	Mat4x4 result;

	for (int row = 0; row < 4; ++row)
	{
		for (int col = 0; col < 4; ++col)
		{
			result.m[col * 4 + row] = m[row * 4 + col];
		}
	}

	return result;
}

constexpr float Mat4x4::Determinant() const
{
	float s0 = m[0] * m[5] - m[4] * m[1];
	float s1 = m[0] * m[6] - m[4] * m[2];
	float s2 = m[0] * m[7] - m[4] * m[3];
	float s3 = m[1] * m[6] - m[5] * m[2];
	float s4 = m[1] * m[7] - m[5] * m[3];
	float s5 = m[2] * m[7] - m[6] * m[3];

	float c0 = m[8] * m[13] - m[12] * m[9];
	float c1 = m[8] * m[14] - m[12] * m[10];
	float c2 = m[8] * m[15] - m[12] * m[11];
	float c3 = m[9] * m[14] - m[13] * m[10];
	float c4 = m[9] * m[15] - m[13] * m[11];
	float c5 = m[10] * m[15] - m[14] * m[11];

	return s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
}

constexpr Mat4x4 Mat4x4::Inverse() const
{
	// Compute 2x2 sub-determinants from the top two rows
	float s0 = m[0] * m[5] - m[4] * m[1];
	float s1 = m[0] * m[6] - m[4] * m[2];
	float s2 = m[0] * m[7] - m[4] * m[3];
	float s3 = m[1] * m[6] - m[5] * m[2];
	float s4 = m[1] * m[7] - m[5] * m[3];
	float s5 = m[2] * m[7] - m[6] * m[3];

	// Compute 2x2 sub-determinants from the bottom two rows
	float c0 = m[8] * m[13] - m[12] * m[9];
	float c1 = m[8] * m[14] - m[12] * m[10];
	float c2 = m[8] * m[15] - m[12] * m[11];
	float c3 = m[9] * m[14] - m[13] * m[10];
	float c4 = m[9] * m[15] - m[13] * m[11];
	float c5 = m[10] * m[15] - m[14] * m[11];

	float det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;

	assert((det > 1e-6f || det < -1e-6f) && "Matrix is not invertible (determinant is ~0).");

	float invDet = 1.0f / det;

	Mat4x4 result;

	result.m[0] = (m[5] * c5 - m[6] * c4 + m[7] * c3) * invDet;
	result.m[1] = (-m[1] * c5 + m[2] * c4 - m[3] * c3) * invDet;
	result.m[2] = (m[13] * s5 - m[14] * s4 + m[15] * s3) * invDet;
	result.m[3] = (-m[9] * s5 + m[10] * s4 - m[11] * s3) * invDet;

	result.m[4] = (-m[4] * c5 + m[6] * c2 - m[7] * c1) * invDet;
	result.m[5] = (m[0] * c5 - m[2] * c2 + m[3] * c1) * invDet;
	result.m[6] = (-m[12] * s5 + m[14] * s2 - m[15] * s1) * invDet;
	result.m[7] = (m[8] * s5 - m[10] * s2 + m[11] * s1) * invDet;

	result.m[8] = (m[4] * c4 - m[5] * c2 + m[7] * c0) * invDet;
	result.m[9] = (-m[0] * c4 + m[1] * c2 - m[3] * c0) * invDet;
	result.m[10] = (m[12] * s4 - m[13] * s2 + m[15] * s0) * invDet;
	result.m[11] = (-m[8] * s4 + m[9] * s2 - m[11] * s0) * invDet;

	result.m[12] = (-m[4] * c3 + m[5] * c1 - m[6] * c0) * invDet;
	result.m[13] = (m[0] * c3 - m[1] * c1 + m[2] * c0) * invDet;
	result.m[14] = (-m[12] * s3 + m[13] * s1 - m[14] * s0) * invDet;
	result.m[15] = (m[8] * s3 - m[9] * s1 + m[10] * s0) * invDet;

	return result;
}

constexpr Mat4x4 Mat4x4::Identity()
{
	Mat4x4 result;

	for (int i = 0; i < 4; ++i)
	{
		result.m[i * 4 + i] = 1.0f;

	}
	return result;
}


constexpr Mat3x3::Mat3x3(float m0, float m1, float m2,
	float m3, float m4, float m5,
	float m6, float m7, float m8) :
	m{ m0, m1, m2, m3, m4, m5, m6, m7, m8 }
{
}

constexpr Mat3x3::Mat3x3(const Vec3 r0, const Vec3 r1, const Vec3 r2)
	: m{ r0.x, r0.y, r0.z,
		r1.x, r1.y, r1.z,
		r2.x, r2.y, r2.z }
{
}

constexpr Mat3x3 Mat3x3::Identity()
{
	return Mat3x3{ 1, 0, 0,
					0, 1, 0,
					0, 0, 1 };
}

constexpr Mat3x3 Mat3x3::Transposed() const
{
	return Mat3x3
	{
		Col(0),
		Col(1),
		Col(2)
	};
}

constexpr float Mat3x3::At(int row, int col) const
{
	assert(0 <= col && col < 3 && 0 <= row && row < 3 && "Mat3x3 index out of bounds.");
	return m[row * 3 + col];
}

constexpr float& Mat3x3::At(int row, int col)
{
	assert(0 <= col && col < 3 && 0 <= row && row < 3 && "Mat3x3 index out of bounds.");
	return m[row * 3 + col];
}

constexpr Vec3 Mat3x3::Row(int index) const
{
	return Vec3(m[index * 3], m[index * 3 + 1], m[index * 3 + 2]);
}

constexpr Vec3 Mat3x3::Col(int index) const
{
	return Vec3(m[index], m[3 + index], m[6 + index]);
}

constexpr Mat3x3 Mat3x3::operator*(const Mat3x3& other) const
{
	Mat3x3 result;
	for (int row = 0; row < 3; ++row)
	{
		for (int col = 0; col < 3; ++col)
		{
			for (int k = 0; k < 3; ++k)
			{
				result.m[row * 3 + col] += m[row * 3 + k] * other.m[k * 3 + col];
			}
		}
	}
	return result;
}


export constexpr Vec3 operator*(const Mat3x3& m, const Vec3 v)
{
	Vec3 result;

	for (int row = 0; row < 3; ++row)
	{
		for (int k = 0; k < 3; ++k)
		{
			result[row] += m.m[row * 3 + k] * v[k];
		}
	}

	return result;
}

export constexpr Vec4 operator*(const Mat4x4& m, const Vec4 v)
{
	Vec4 result;

	for (int row = 0; row < 4; ++row)
	{
		for (int k = 0; k < 4; ++k)
		{
			result[row] += m.m[row * 4 + k] * v[k];
		}
	}

	return result;
}

constexpr float Vec2::operator[](int index) const
{
	assert(0 <= index && index < 2 && "Vec2 index out of bounds");
	return index == 0 ? x : y;
}

constexpr float& Vec2::operator[](int index)
{
	assert(0 <= index && index < 2 && "Vec2 index out of bounds");
	return index == 0 ? x : y;
}

constexpr Vec2& Vec2::operator+=(const Vec2 other)
{
	x += other.x;
	y += other.y;
	return *this;
}

constexpr Vec2& Vec2::operator-=(const Vec2 other)
{
	x -= other.x;
	y -= other.y;
	return *this;
}

constexpr Vec2& Vec2::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	return *this;
}

constexpr Vec2& Vec2::operator/=(float scalar)
{
	x /= scalar;
	y /= scalar;
	return *this;
}

constexpr Vec2 Vec2::Up()
{
	return Vec2(0.0f, 1.0f);
}

constexpr Vec2 Vec2::Right()
{
	return Vec2(1.0f, 0.0f);
}

constexpr Vec2 Vec2::Down()
{
	return Vec2(0.0f, -1.0f);
}

constexpr Vec2 Vec2::Left()
{
	return Vec2(-1.0f, 0.0f);
}

void Vec2::Normalize()
{
	assert(SqrMagnitude() > 0.0f && "Cannot normalize a zero vector.");

	float len = Length();
	x = x / len;
	y = y / len;
}

void Vec2::NormalizeSafe(const Vec2 fallback)
{
	float len = Length();
	if (len > 0.0f)
	{
		x = x / len;
		y = y / len;
	}
	else
	{
		x = fallback.x;
		y = fallback.y;
	}
}

Vec2 Vec2::Normalized() const
{
	assert(SqrMagnitude() > 0.0f && "Cannot normalize a zero vector.");

	float len = Length();
	return Vec2(x / len, y / len);
}

Vec2 Vec2::NormalizedSafe(const Vec2 fallback) const
{
	if (float len = Length(); len > 0.0f)
	{
		return Vec2(x / len, y / len);
	}

	return fallback;
}

float Vec2::Length() const
{
	return std::hypotf(x, y);
}

constexpr float Vec2::SqrMagnitude() const
{
	return x * x + y * y;
}

constexpr float Vec2::Dot(const Vec2 other) const
{
	return x * other.x + y * other.y;
}

constexpr Vec2 Vec2::operator+(const Vec2 other) const
{
	return Vec2(x + other.x, y + other.y);
}

constexpr Vec2 Vec2::operator-(const Vec2 other) const
{
	return Vec2(x - other.x, y - other.y);
}

constexpr Vec2 Vec2::operator*(float scalar) const
{
	return Vec2(x * scalar, y * scalar);
}

constexpr Vec2 Vec2::operator/(float scalar) const
{
	return Vec2(x / scalar, y / scalar);
}

constexpr Vec2 Vec2::operator-() const
{
	return Vec2(-x, -y);
}

// t is unclamped; values outside [0, 1] will extrapolate
export constexpr Vec2 Lerp(const Vec2 a, const Vec2 b, float t) noexcept
{
	return Vec2(std::lerp(a.x, b.x, t), std::lerp(a.y, b.y, t));
}

export constexpr Vec2 operator*(float scalar, const Vec2 v) noexcept { return v * scalar; }

template<>
struct std::formatter<Vec2>
{
private:
	mutable std::formatter<float> _floatFormatter;

public:
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return _floatFormatter.parse(ctx);
	}

	auto format(Vec2 vec, std::format_context& ctx) const
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

constexpr Vec3::Vec3(const Vec2 other) : x{ other.x }, y{ other.y } {}

constexpr Vec3::Vec3(const Vec2 xy, float z_) : x{ xy.x }, y{ xy.y }, z{ z_ } {}

constexpr Vec2 Vec3::xy() const
{
	return Vec2(x, y);
}

constexpr Vec2 Vec3::xz() const
{
	return Vec2(x, z);
}

constexpr Vec2 Vec3::yz() const
{
	return Vec2(y, z);
}

constexpr float Vec3::operator[](int index) const
{
	assert(0 <= index && index < 3 && "Vec3 index out of bounds");
	if (index == 0) return x;
	if (index == 1) return y;
	return z;
}

constexpr float& Vec3::operator[](int index)
{
	assert(0 <= index && index < 3 && "Vec3 index out of bounds");
	if (index == 0) return x;
	if (index == 1) return y;
	return z;
}

constexpr Vec3& Vec3::operator+=(const Vec3 other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

constexpr Vec3& Vec3::operator-=(const Vec3 other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

constexpr Vec3& Vec3::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

constexpr Vec3& Vec3::operator/=(float scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
	return *this;
}

constexpr Vec3 Vec3::operator+(const Vec3 other) const
{
	return Vec3(x + other.x, y + other.y, z + other.z);
}

constexpr Vec3 Vec3::operator-(const Vec3 other) const
{
	return Vec3(x - other.x, y - other.y, z - other.z);
}

constexpr Vec3 Vec3::operator*(float scalar) const
{
	return Vec3(x * scalar, y * scalar, z * scalar);
}

constexpr Vec3 Vec3::operator/(float scalar) const
{
	return Vec3(x / scalar, y / scalar, z / scalar);
}

constexpr Vec3 Vec3::operator-() const
{
	return Vec3(-x, -y, -z);
}

constexpr float Vec3::Dot(const Vec3 other) const
{
	return x * other.x + y * other.y + z * other.z;
}

constexpr Vec3 Vec3::Cross(const Vec3 other) const
{
	return Vec3(
		y * other.z - z * other.y,
		z * other.x - x * other.z,
		x * other.y - y * other.x
	);
}

constexpr float Vec3::SqrMagnitude() const
{
	return x * x + y * y + z * z;
}

float Vec3::Length() const
{
	return std::sqrtf(x * x + y * y + z * z);
}

constexpr Vec3 Vec3::Up() { return Vec3(0.0f, 1.0f, 0.0f); }
constexpr Vec3 Vec3::Right() { return Vec3(1.0f, 0.0f, 0.0f); }
constexpr Vec3 Vec3::Down() { return Vec3(0.0f, -1.0f, 0.0f); }
constexpr Vec3 Vec3::Left() { return Vec3(-1.0f, 0.0f, 0.0f); }
constexpr Vec3 Vec3::Forward() { return Vec3(0.0f, 0.0f, 1.0f); }
constexpr Vec3 Vec3::Back() { return Vec3(0.0f, 0.0f, -1.0f); }

void Vec3::Normalize()
{
	assert(SqrMagnitude() > 0.0f && "Cannot normalize a zero vector.");
	float len = Length();
	x /= len;
	y /= len;
	z /= len;
}

void Vec3::NormalizeSafe(const Vec3 fallback)
{
	if (float len = Length(); len > 0.0f)
	{
		x /= len;
		y /= len;
		z /= len;
	}
	else
	{
		x = fallback.x;
		y = fallback.y;
		z = fallback.z;
	}
}

Vec3 Vec3::Normalized() const
{
	assert(SqrMagnitude() > 0.0f && "Cannot normalize a zero vector.");
	float len = Length();
	return Vec3(x / len, y / len, z / len);
}

Vec3 Vec3::NormalizedSafe(const Vec3 fallback) const
{
	if (float len = Length(); len > 0.0f)
		return Vec3(x / len, y / len, z / len);

	return fallback;
}

// t is not clamped; values outside [0, 1] will extrapolate
export constexpr Vec3 Lerp(const Vec3 a, const Vec3 b, float t) noexcept
{
	return Vec3(std::lerp(a.x, b.x, t),
		std::lerp(a.y, b.y, t),
		std::lerp(a.z, b.z, t));
}

export constexpr Vec3 operator*(float scalar, const Vec3 v) noexcept { return v * scalar; }

template<>
struct std::formatter<Vec3>
{
private:
	mutable std::formatter<float> _floatFormatter;

public:
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return _floatFormatter.parse(ctx);
	}

	auto format(Vec3 vec, std::format_context& ctx) const
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

constexpr Vec4::Vec4(const Vec3 vec3, float w_) : x{ vec3.x }, y{ vec3.y }, z{ vec3.z }, w{ w_ }
{
}

constexpr Vec4::Vec4(const Vec2 xy, const Vec2 zw) : x{ xy.x }, y{ xy.y }, z{ zw.x }, w{ zw.y }
{
}

constexpr Vec3 Vec4::xyz() const { return Vec3(x, y, z); }
constexpr Vec2 Vec4::xy()  const { return Vec2(x, y); }
constexpr Vec2 Vec4::zw()  const { return Vec2(z, w); }

constexpr float Vec4::operator[](int index) const
{
	assert(0 <= index && index < 4 && "Vec4 index out of bounds");
	if (index == 0) return x;
	if (index == 1) return y;
	if (index == 2) return z;
	return w;
}

constexpr float& Vec4::operator[](int index)
{
	assert(0 <= index && index < 4 && "Vec4 index out of bounds");
	if (index == 0) return x;
	if (index == 1) return y;
	if (index == 2) return z;
	return w;
}

constexpr Vec4& Vec4::operator+=(const Vec4 other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	w += other.w;
	return *this;
}

constexpr Vec4& Vec4::operator-=(const Vec4 other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	w -= other.w;
	return *this;
}

constexpr Vec4& Vec4::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
	return *this;
}

constexpr Vec4& Vec4::operator/=(float scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
	w /= scalar;
	return *this;
}

constexpr Vec4 Vec4::operator+(const Vec4 other) const
{
	return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
}

constexpr Vec4 Vec4::operator-(const Vec4 other) const
{
	return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
}

constexpr Vec4 Vec4::operator*(float scalar) const
{
	return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
}

constexpr Vec4 Vec4::operator/(float scalar) const
{
	return Vec4(x / scalar, y / scalar, z / scalar, w / scalar);
}

constexpr Vec4 Vec4::operator-() const
{
	return Vec4(-x, -y, -z, -w);
}

constexpr float Vec4::Dot(const Vec4 other) const
{
	return x * other.x + y * other.y + z * other.z + w * other.w;
}

constexpr float Vec4::SqrMagnitude() const
{
	return x * x + y * y + z * z + w * w;
}

float Vec4::Length() const
{
	return std::sqrtf(x * x + y * y + z * z + w * w);
}

void Vec4::Normalize()
{
	assert(SqrMagnitude() > 0.0f && "Cannot normalize a zero vector.");
	float len = Length();
	x /= len;
	y /= len;
	z /= len;
	w /= len;
}

void Vec4::NormalizeSafe(const Vec4 fallback)
{
	if (float len = Length(); len > 0.0f)
	{
		x /= len;
		y /= len;
		z /= len;
		w /= len;
	}
	else
	{
		x = fallback.x;
		y = fallback.y;
		z = fallback.z;
		w = fallback.w;
	}
}

Vec4 Vec4::Normalized() const
{
	assert(SqrMagnitude() > 0.0f && "Cannot normalize a zero vector.");
	float len = Length();
	return Vec4(x / len, y / len, z / len, w / len);
}

Vec4 Vec4::NormalizedSafe(const Vec4 fallback) const
{
	if (float len = Length(); len > 0.0f)
		return Vec4(x / len, y / len, z / len, w / len);

	return fallback;
}

// t is not clamped; values outside [0, 1] will extrapolate
export constexpr Vec4 Lerp(const Vec4 a, const Vec4 b, float t) noexcept
{
	return Vec4(std::lerp(a.x, b.x, t),
		std::lerp(a.y, b.y, t),
		std::lerp(a.z, b.z, t),
		std::lerp(a.w, b.w, t));
}

export constexpr Vec4 operator*(float scalar, const Vec4 v) noexcept { return v * scalar; }

template<>
struct std::formatter<Vec4>
{
private:
	mutable std::formatter<float> _floatFormatter;

public:
	constexpr auto parse(std::format_parse_context& ctx)
	{
		return _floatFormatter.parse(ctx);
	}

	auto format(Vec4 vec, std::format_context& ctx) const
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

Quaternion::Quaternion(const Mat3x3& matrix)
{
	// Fletcher - Dunn p286
	// 0 1 2
	// 3 4 5
	// 6 7 8

	const auto& m = matrix.m;

	const float fourWSquaredMinus1 = m[0] + m[4] + m[8];
	const float fourXSquaredMinus1 = m[0] - m[4] - m[8];
	const float fourYSquaredMinus1 = m[4] - m[0] - m[8];
	const float fourZSquaredMinus1 = m[8] - m[0] - m[4];

	int biggestIndex = 0;
	float fourBiggest = fourWSquaredMinus1;
	if (fourXSquaredMinus1 > fourBiggest) { fourBiggest = fourXSquaredMinus1; biggestIndex = 1; }
	if (fourYSquaredMinus1 > fourBiggest) { fourBiggest = fourYSquaredMinus1; biggestIndex = 2; }
	if (fourZSquaredMinus1 > fourBiggest) { fourBiggest = fourZSquaredMinus1; biggestIndex = 3; }

	const float biggestVal = sqrtf(fourBiggest + 1.f) * 0.5f;
	const float mult = 0.25f / biggestVal;

	switch (biggestIndex)
	{
	case 0:
		w = biggestVal;
		x = (m[5] - m[7]) * mult;
		y = (m[6] - m[2]) * mult;
		z = (m[1] - m[3]) * mult;
		break;

	case 1:
		w = (m[5] - m[7]) * mult;
		x = biggestVal;
		y = (m[1] + m[3]) * mult;
		z = (m[6] + m[2]) * mult;
		break;

	case 2:
		w = (m[6] - m[2]) * mult;
		x = (m[1] + m[3]) * mult;
		y = biggestVal;
		z = (m[5] + m[7]) * mult;
		break;

	case 3:
		w = (m[1] - m[3]) * mult;
		x = (m[6] + m[2]) * mult;
		y = (m[5] + m[7]) * mult;
		z = biggestVal;
		break;
	}
}

// Returns the conjugate. Only valid as the true inverse for unit quaternions.
constexpr Quaternion Quaternion::Inverse() const
{
	return Quaternion(w, -x, -y, -z);
}

Quaternion Quaternion::AngleAxis(float angleDegrees, Vec3 axis)
{
	const float alpha = (angleDegrees / 2.0f) * (PI / 180.f);
	const float w = cosf(alpha);
	axis *= sinf(alpha);
	return Quaternion(w, axis.x, axis.y, axis.z);
}

Quaternion Quaternion::LookRotation(Vec3 targetDirection, Vec3 targetRight, Vec3 targetUp)
{
	const Mat3x3 m(targetRight, targetUp, targetDirection);
	return Quaternion(m);
}

Quaternion Quaternion::Euler(Vec3 degrees, RotateOrder order)
{
	const float pitchHalf = degrees.x / 2.0f * TO_RAD;
	const float yawHalf = degrees.y / 2.0f * TO_RAD;
	const float rollHalf = degrees.z / 2.0f * TO_RAD;

	const Quaternion qPitch(cosf(pitchHalf), sinf(pitchHalf), 0.0f, 0.0f);
	const Quaternion qYaw(cosf(yawHalf), 0.0f, sinf(yawHalf), 0.0f);
	const Quaternion qRoll(cosf(rollHalf), 0.0f, 0.0f, sinf(rollHalf));

	switch (order)
	{
	case RotateOrder::RollPitchYaw: return qRoll * qPitch * qYaw;
	case RotateOrder::RollYawPitch: return qRoll * qYaw * qPitch;

	case RotateOrder::PitchYawRoll: return qPitch * qYaw * qRoll;
	case RotateOrder::PitchRollYaw: return qPitch * qRoll * qYaw;

	case RotateOrder::YawRollPitch: return qYaw * qRoll * qPitch;

	default:
		return qYaw * qPitch * qRoll;
	}
}

Quaternion Quaternion::Euler(float pitch, float yaw, float roll, RotateOrder order)
{
	const float pitchAngleHalf = (pitch / 2.0f) * TO_RAD;
	const float yawAngleHalf = (yaw / 2.0f) * TO_RAD;
	const float rollAngleHalf = (roll / 2.0f) * TO_RAD;

	const Quaternion qPitch(cosf(pitchAngleHalf), sinf(pitchAngleHalf), 0.0f, 0.0f);
	const Quaternion qYaw(cosf(yawAngleHalf), 0.0f, sinf(yawAngleHalf), 0.0f);
	const Quaternion qRoll(cosf(rollAngleHalf), 0.0f, 0.0f, sinf(rollAngleHalf));

	switch (order)
	{
	case RotateOrder::RollPitchYaw: return qRoll * qPitch * qYaw;
	case RotateOrder::RollYawPitch: return qRoll * qYaw * qPitch;

	case RotateOrder::PitchYawRoll: return qPitch * qYaw * qRoll;
	case RotateOrder::PitchRollYaw: return qPitch * qRoll * qYaw;

	case RotateOrder::YawRollPitch: return qYaw * qRoll * qPitch;
	default:
		return qYaw * qPitch * qRoll;
	}
}

constexpr Quaternion Quaternion::FromTo(const Quaternion& from, const Quaternion& to)
{
	return to - from;
}

float Quaternion::Magnitude() const
{
	return sqrtf(x * x + y * y + z * z + w * w);
}

constexpr Quaternion Quaternion::operator*(const Quaternion& other) const
{
	//"p" as in "product"
	float pW = (w * other.w) - (x * other.x + y * other.y + z * other.z);

	const Vec3 v1{ x, y, z };
	const Vec3 v2{ other.x, other.y, other.z };

	const Vec3 pV = (v2 * w) + (v1 * other.w) + v1.Cross(v2);

	return Quaternion(pW, pV);
}

// Returns the relative rotation from 'other' to 'this' (i.e. this * other.Inverse())
// NOT component-wise subtraction.
constexpr Quaternion Quaternion::operator-(const Quaternion& other) const
{
	return (*this) * other.Inverse();
}

constexpr Quaternion Quaternion::operator*(float scalar) const
{
	return Quaternion(w * scalar, x * scalar, y * scalar, z * scalar);
}

constexpr Quaternion& Quaternion::operator/=(float scalar)
{
	w /= scalar;
	x /= scalar;
	y /= scalar;
	z /= scalar;

	return *this;
}

constexpr Quaternion& Quaternion::operator+=(const Quaternion& other)
{
	w += other.w;
	x += other.x;
	y += other.y;
	z += other.z;

	return *this;
}

Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float t, bool forceLerp)
{
	// Courtesy of Fletcher & Dunn p262
	const float w0 = a.w;
	const float x0 = a.x;
	const float y0 = a.y;
	const float z0 = a.z;

	float w1 = b.w;
	float x1 = b.x;
	float y1 = b.y;
	float z1 = b.z;

	float cosOmega = w0 * w1 + x0 * x1 + y0 * y1 + z0 * z1;

	if (cosOmega < 0.0f)
	{
		cosOmega = -cosOmega;

		w1 = -w1;
		x1 = -x1;
		y1 = -y1;
		z1 = -z1;
	}

	float k0{ 0.f };
	float k1{ 0.f };

	if (forceLerp || cosOmega > 0.9999f)
	{
		k0 = 1.0f - t;
		k1 = t;
	}
	else
	{
		float sinOmega = sqrtf(1.0f - cosOmega * cosOmega);
		float omega = atan2(sinOmega, cosOmega);
		float oneOverSinOmega = 1.0f / sinOmega;

		k0 = sin((1.0f - t) * omega) * oneOverSinOmega;
		k1 = sin(t * omega) * oneOverSinOmega;
	}

	return Quaternion(w0 * k0 + w1 * k1,
		x0 * k0 + x1 * k1,
		y0 * k0 + y1 * k1,
		z0 * k0 + z1 * k1);
}

constexpr void Mat4x4::SetRow(int index, float x, float y, float z, float w)
{
	assert(0 <= index && index < 4 && "Matrix row out of bounds.");

	const int r = index * 4;
	m[r] = x;
	m[r + 1] = y;
	m[r + 2] = z;
	m[r + 3] = w;
}

constexpr Mat4x4 Mat4x4::Scale(Vec3 scale)
{
	return Mat4x4::Scale(scale.x, scale.y, scale.z);
}

constexpr Mat4x4 Mat4x4::Scale(float x, float y, float z)
{
	Mat4x4 s;
	s.m[0] = x;
	s.m[5] = y;
	s.m[10] = z;
	return s;
}

constexpr Mat4x4 Mat4x4::Translate(Vec3 position)
{
	Mat4x4 t;
	t.m[3] = position.x;
	t.m[7] = position.y;
	t.m[11] = position.z;
	return t;
}

constexpr Mat4x4 Mat4x4::TRS(Vec3 translation, const Quaternion& rotation, Vec3 scale)
{
	const Mat4x4 s = Scale(scale.x, scale.y, scale.z);
	const Mat4x4 r = FromQuaternion(rotation.w, rotation.x, rotation.y, rotation.z);

	Mat4x4 trs = r * s;

	// Translation is set directly in the last column, independently
	// of rotation and scale.
	trs.m[3] = translation.x;
	trs.m[7] = translation.y;
	trs.m[11] = translation.z;

	return trs;
}

constexpr Mat4x4 Mat4x4::FromQuaternion(float w, float x, float y, float z)
{
	float x2 = x * x;	float y2 = y * y;	float z2 = z * z;

	Mat4x4 result;
	auto& m = result.m;

	m[0] = 1.0f - 2.0f * (y2 + z2);
	m[1] = 2.0f * x * y + 2.0f * w * z;
	m[2] = 2.0f * x * z - 2.0f * w * y;
	m[3] = 0.0f;

	m[4] = 2.0f * x * y - 2.0f * w * z;
	m[5] = 1.0f - 2.0f * (x2 + z2);
	m[6] = 2.0f * y * z + 2.0f * w * x;
	m[7] = 0.0f;

	m[8] = 2.0f * x * z + 2.0f * w * y;
	m[9] = 2.0f * y * z - 2.0f * w * x;
	m[10] = 1.0f - 2.0f * (x2 + y2);
	m[11] = 0.0f;

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;

	return result;
}
