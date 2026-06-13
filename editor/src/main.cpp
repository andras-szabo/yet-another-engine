#include <array>
#include <chrono>
#include <iostream>
#include <span>
#include <string>
#include <cassert>
#include <thread>
#include "LoggerMacros.h"

// Import the engine-core module.
// Once engine-core exports real symbols, they will be available here.
#if defined ( __INTELLISENSE__ )
#include "../../engine-core/src/EngineError.ixx"
#include "../../engine-core/src/EngineInstance.ixx"
#include "../../engine-core/src/FileWatcher.ixx"
#include "../../engine-core/src/Logger.ixx"
#include "../../engine-core/src/Math.ixx"
#include "../../engine-core/src/Reflection.ixx"
#include "../../engine-core/src/Scene.ixx"
#include "../../engine-core/src/Serialization.ixx"
#include "EditorTests.ixx"
#else
import EngineCore;
import EngineInstance;
import EditorTests;
#endif

using namespace Engine;

Engine::Expected<int> ShouldBeEven(int number)
{
    if (number % 2 == 0)
    {
        return number;
    }

    return std::unexpected<Engine::Error>({Engine::ErrorType::Logic, "Bloargh"});
}

void TestM3x3()
{
    Engine::Vec3 a{ 1, 2, 3 };
    Engine::Vec3 b = Engine::Mat3x3::Identity() * a;
    LOG_INFO("[Foo] identity * a = {}", b);
}

void TestVec4()
{
    using namespace Engine;
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
    using namespace Engine;
    Vec3 a;
    Vec3 b{ 1, 2.23f, 3 };
    Vec3 c = a + b;
    LOG_INFO("[Foo] Vec3: a: {}, b: {}, a + b = {:.2f}", a, b, c);
}

void TestVec2()
{
    using namespace Engine;
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

void TestDF()
{
    LOG_INFO("Starting datafile test ---");

    using namespace Engine;
    DataFile df;

    df["Foo"] = 1234.f;
    df["Bar"] = 42;
    df["Baz"] = "Baz";

    Vec2 v{ -123.4f, 3.14f };
    df["Position"] = v;

    Vec2 vCopy{ df["Position"] };

    LOG_INFO("Df with assignment ops: {}", df.ToString());
    LOG_INFO("Df deserialized: {}", vCopy);
}

struct NestedType
{
    int john{ 0 };
    float paul{ 0.f };
    Engine::Vec3 george;
    Engine::Quaternion ringo;
};

std::span<const Engine::FieldDescriptor> GetNestedTypeFieldDescriptors()
{
    static Engine::FieldDescriptor nt_john{ "john", Engine::FieldType::Int, offsetof(NestedType, john) };
    static Engine::FieldDescriptor nt_paul{ "paul", Engine::FieldType::Float, offsetof(NestedType, paul) };
    static Engine::FieldDescriptor nt_george{ "george", Engine::FieldType::Vec3, offsetof(NestedType, george) };
    static Engine::FieldDescriptor nt_ringo{ "ringo", Engine::FieldType::Quaternion, offsetof(NestedType, ringo) };

    static std::array<Engine::FieldDescriptor, 4> fields
    {
        nt_john, nt_paul, nt_george, nt_ringo
    };

    return fields;
}

void TestSrsly()
{
    struct TestType
    {
        int foo;
        int bar;
        float baz;
        NestedType fizz;
    };

    Engine::FieldDescriptor f_foo{ "foo", Engine::FieldType::Int, offsetof(TestType, foo) };
    Engine::FieldDescriptor f_bar{ "bar", Engine::FieldType::Int, offsetof(TestType, bar) };
    Engine::FieldDescriptor f_baz{ "baz", Engine::FieldType::Float, offsetof(TestType, baz) };
    Engine::FieldDescriptor f_fizz{ "fizz", Engine::FieldType::Composite, offsetof(TestType, fizz), GetNestedTypeFieldDescriptors };

    std::array<Engine::FieldDescriptor, 4> fields
    {
        f_foo, f_bar, f_baz, f_fizz
    };

    TestType t;
    t.foo = 123;
    t.bar = -234;
    t.baz = 45.424f;
    t.fizz.john = 99;
    t.fizz.paul = -42.f;
    t.fizz.george = { -5.0f, -6.0f, -7.0f };
    t.fizz.ringo = Engine::Quaternion::Identity();

    Engine::DataFile df{};

    /*Engine::SerializeFields(&t, fields, df);
    LOG_INFO("Serialization at work:\n{}", df.ToString());

    LOG_INFO("And now to deserialize...");
    TestType read;
    Engine::DeserializeFields(&read, fields, df);
    LOG_INFO("Deserialized: foo = {}, bar = {}, baz = {}", read.foo, read.bar, read.baz);
    */
}

void RunTests()
{
    EditorTests::RunSceneTest();
}

void RunFileWatcherTest()
{
    Engine::FileWatcher watcher(L"C:\\Users\\andra", L"todo.txt");

    if (watcher.IsValid())
    {
        LOG_INFO("FileWatcher is valid.");

        int waitForSeconds = 30;

        while (waitForSeconds --> 0)
        {
            if (watcher.Poll())
            {
                LOG_INFO("Yup, the file was just modified.");
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        
    }
    else
    {
        LOG_WARNING("FileWatcher is not valid.");
    }
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
    TestDF();
    TestSrsly();

    for (int i = 0; i < 5; ++i)
    {
        LOG_INFO("Creating a guid: {}", Engine::GUID{});
    }

    
    Engine::Instance.Initialize(std::make_unique<Engine::ComponentStorage>());

    LOG_INFO("-----");
    RunTests();
    LOG_INFO("-----");

    auto g = Engine::Instance
        .GetActiveScene()
        .CreateGameObject(&Engine::Instance.GetComponentStorage(), "Foo", 0);

    g->GetName();

    assert(g->GetTransform() != nullptr && "GameObject created but has no transform.");
    LOG_INFO("G has a transform, yay!");
    LOG_INFO("Active scene: {}", Engine::Instance.GetActiveScene().GetSceneName());

    // Let's walk through it...
    auto logNodes = [](Engine::Scene::Scene& scene, std::size_t currentIndex) {
        LOG_INFO("Index: {}, name: {}", currentIndex, scene.GetNodeName(currentIndex));
        };

    auto& scene = Engine::Instance.GetActiveScene();

    scene.WalkBreadthFirst(0, logNodes);

    auto firstChild = scene.CreateGameObject(&Engine::Instance.GetComponentStorage(),
        "FirstChild", g->GetTransform()->GetSceneNodeIndex());

    LOG_INFO(scene.GetNodeName(firstChild->GetTransform()->GetSceneNodeIndex()));

    // Add some nodes to the root
    for (int i = 0; i < 3; ++i)
    {
        scene.CreateGameObject(&Engine::Instance.GetComponentStorage(),
            std::format("child_{}", i),
            g->GetTransform()->GetSceneNodeIndex());
    }

    scene.WalkBreadthFirst(0, logNodes);

    // Add a nested hierarchy to the first child
    int parentIndex = firstChild->GetTransform()->GetSceneNodeIndex();
    auto cs = &Engine::Instance.GetComponentStorage();
    for (int i = 0; i < 3; ++i)
    {
        const std::string name = std::format("nested_child_{}", i);
        auto newParent = scene.CreateGameObject(cs, name, parentIndex);
        parentIndex = newParent->GetTransform()->GetSceneNodeIndex();
    }

    LOG_INFO("--- \n\n ---");

    LOG_INFO("Depth-first walk");
    scene.WalkDepthFirst(0, logNodes);

    LOG_INFO("Breadth-first walk");
    scene.WalkBreadthFirst(0, logNodes);

    scene.UpdateWorldTransforms();

    LOG_INFO("World transforms updated");

    Engine::DataFile fOut;
    Engine::SerializeScene(scene, fOut);

    LOG_INFO("Serialized scene: {}", fOut.ToString());

    // And now,, the deserialization... o.O
    auto sceneMaybe = Engine::DeserializeScene(fOut, Engine::Instance.GetComponentStorage());
    if (!sceneMaybe.has_value())
    {
        LOG_ERROR("Uh-oh. {}", sceneMaybe.error().message);
    }
    else
    {
        LOG_INFO("My word. We deserialized a scene! Let's try to log it! \n\n");
        auto& newScene = sceneMaybe.value();
        LOG_INFO("Break here...");
        newScene.WalkDepthFirst(0, logNodes);
    }

    //RunFileWatcherTest();

    // Try to add another transform; this should fail.
    //g.AddComponent<Engine::Transform>();

    /*Engine::Transform* addedComponent = g.AddComponent<Engine::Transform>();
    assert(addedComponent != nullptr && "Couldn't add transform");

    LOG_INFO("Added trsf, and its value is {}", (std::size_t)addedComponent);

    Engine::Transform* gotComponent = g.GetComponent<Engine::Transform>();
    assert(addedComponent == gotComponent && "Couldn't get transform");

    LOG_INFO("Yay we could get trsf, and its value is {}", (std::size_t) gotComponent);*/

    return 0;
}
