#include <iostream>
#include <string>
#include "LoggerMacros.h"

// Import the engine-core module.
// Once engine-core exports real symbols, they will be available here.
import EngineCore;

Expected<int> ShouldBeEven(int number)
{
    if (number % 2 == 0)
    {
        return number;
    }

    return std::unexpected<EngineError>({ErrorType::LogicError, "Bloargh"});
}

void TestM3x3()
{
    Vec3 a{ 1, 2, 3 };
    Vec3 b = Mat3x3::Identity() * a;
    LOG_INFO("[Foo] identity * a = {}", b);
}

void TestVec4()
{
    Vec4 a;
    Vec4 b { 1, 2, 3, 0 };
    Vec4 c { 5, 6, 7, 8 };
    LOG_INFO("[Foo] Vec4: a: {}, b: {}, b + c = {}", a, b, b + c);

    Vec4 d;
    Vec4 e{ 10, 9, 8, 7 };
    LOG_INFO("[Foo] Lerp 0.25: {} // 0.75: {}", Lerp(d, e, 0.25f), Lerp(d, e, 0.75f));
}

void TestVec3()
{
    Vec3 a;
    Vec3 b{ 1, 2, 3 };
    Vec3 c = a + b;
    LOG_INFO("[Foo] Vec3: a: {}, b: {}, a + b = {:.2f}", a, b, c);
}

void TestVec2()
{
    constexpr Vec2 a{};
    static_assert(a.x == 0.0f && a.y == 0.0f);

    constexpr Vec2 b{ 1, 2 };
    static_assert(b.x == 1.0f && b.y == 2.0f);

    Vec2 d = Vec2::Right();
    d += Vec2::Left();
    d -= Vec2::Right();

    LOG_INFO("[Foo] right + left - right = {}", d);

    Vec2 e = Vec2::Up() * 12.3456f;
    LOG_INFO("[Foo] Up * 12.3456f = {:.2f}", e);

    LOG_INFO("[Foo] 'e' normalized = {}", e.Normalized());

    Vec2 f{ 10, 10 };
    Vec2 g{ 1, 0 };
    LOG_INFO("[Foo] 'f.g' = {}", f.Dot(g));

    Vec2 h{ 123, 456 };
    LOG_INFO("[Foo] Before: {}, {}", h[0], h[1]);
   
    h[0] = 999.0f;
    h[1] = -42.0f;
    LOG_INFO("[Foo] After: {}", h);

    LOG_INFO("Vec2 tests run.");
}

int main()
{
    LOG_INFO("This is an info");
    auto bar = ShouldBeEven(42);
    auto baz = ShouldBeEven(3);

    if (bar.has_value())
    {
        LOG_INFO("OK, bar checks out!, and its value is {}", bar.value());
    }

    if (!baz.has_value())
    {
        LOG_ERROR("Oho. {}", baz.error().message);
    }

    TestVec2();
    TestVec3();
    TestVec4();
    TestM3x3();

    for (int i = 0; i < 5; ++i)
    {
        LOG_INFO("Creating a guid: {}", GUID{});
    }

    return 0;
}
