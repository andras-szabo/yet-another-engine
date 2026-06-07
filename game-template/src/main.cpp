#include <iostream>

// TODO: make this simpler using just 1 include file
#include "../../engine-core/include/ReflectionMacros.h"
#include "../../engine-core/include/LoggerMacros.h"

// Import the engine-core module.
// Once engine-core exports real symbols, they will be available here.
#if defined ( __INTELLISENSE__ )
#include "../../engine-core/src/ComponentStorage.ixx"
#include "../../engine-core/src/IComponentStorage.ixx"
#include "../../engine-core/src/EngineError.ixx"
#include "../../engine-core/src/EngineInstance.ixx"
#include "../../engine-core/src/Logger.ixx"
#include "../../engine-core/src/Math.ixx"
#include "../../engine-core/src/Reflection.ixx"
#include "../../engine-core/src/Scene.ixx"
#include "../../engine-core/src/Serialization.ixx"
#else
import EngineCore;
import EngineInstance;
#endif

//using namespace Engine;

class MyComponent : public Engine::Component
{
    COMPONENT_BODY(MyComponent)
    REGISTER_COMPONENT(MyComponent)

public:
    MyComponent() = default;
    ~MyComponent() override = default;

    void SetFoo(int f) { _foo = f; }

private:
    int _foo;
    float _bar;
    Engine::Vec3 _baz;
};

REFLECTED_FIELDS(MyComponent,
    { "_foo", Engine::FieldType::Int, offsetof(MyComponent, _foo) })


int main()
{
    std::cout << "[game-template] stub — game loop will go here\n";

    LOG_INFO("Initializing...");
    Engine::Instance.Initialize(std::make_unique<Engine::ComponentStorage>());
    auto& scene = Engine::Instance.GetActiveScene();
    auto g = scene.CreateGameObject(&Engine::Instance.GetComponentStorage(),
        "FooContainer",
        0);

    auto* mc = g->AddComponent<MyComponent>(&Engine::Instance.GetComponentStorage());
    mc->SetFoo(123);

    Engine::DataFile fOut;
    Engine::SerializeScene(scene, fOut);
    LOG_INFO("Serialized scene:\n{}", fOut.ToString());

    return 0;
}

