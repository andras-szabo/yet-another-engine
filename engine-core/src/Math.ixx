module;

#include <cassert>
#include <cmath>
#include <format>

#include "engine_core_api.h"

export module Math;

export struct ENGINE_CORE_API Vec2
{
	float x;
	float y;

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

export struct Vec3
{
	float x;
	float y;
	float z;
};

export struct Vec4
{
	float x;
	float y;
	float z;
	float w;
};

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
