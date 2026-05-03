module;

#include <cassert>
#include <cmath>
#include <format>

#include "engine_core_api.h"

export module Math;

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
