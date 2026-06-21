#pragma once

#include <cstddef>      // offsetof
#include <functional>
#include <memory>
#include <span>
#include <string_view>

// Companion header for the Reflection module.
// Macros cannot be exported from C++ modules, so they live here.
//
// Usage in a Component-derived module unit (.ixx):
//
//   module;
//   #include "ReflectionMacros.h"   // <-- in the global module fragment
//   ...
//   export module MyComponent;
//   import Component;
//   import Reflection;
//
//   export class MyComponent : public Component
//   {
//       COMPONENT_BODY(MyComponent)
//   public:
//       FIELD() float   speed;
//       FIELD() Vec3    direction;
//   };
//
//   Reflected fields (hand written now, code gen later):
//  
//   REFLECTED_FIELDS(MyComponent,
//       { "speed",     FieldType::Float, offsetof(MyComponent, speed)     },
//       { "direction", FieldType::Vec3,  offsetof(MyComponent, direction) }
//   )
//
// If a component has no reflected fields use REFLECTED_FIELDS_EMPTY instead:
//
//   REFLECTED_FIELDS_EMPTY(MyComponent)
//
// ----------------------------------------------------------------------------
//
// FIELD(...) — passive annotation.  Expands to nothing at compile time.
// A future code-generation tool will parse these markers and produce the
// corresponding .reflected.h file automatically.
// Optional arguments (e.g. FIELD(EditAnywhere)) are accepted and ignored.
#define FIELD(...)


// COMPONENT_BODY(ClassName) — place as the first line inside a
// Component-derived class body.  Declares:
//   - TypeName           : compile-time string_view of the class name
//   - GetTypeName()      : static accessor for TypeName
//   - GetFieldDescriptors() : static function defined in ClassName.reflected.h
//   - GetReflectedFields()  : virtual override that delegates to the static
//                             function, enabling runtime access via Component*
// ! GetReflectedFields() shall not be defined inline! REFLECTED_FIELDS shall be
// placed in a corresponding .cpp file, after REGISTER_COMPONENT.

#define COMPONENT_BODY(ClassName)                                                   \
public:                                                                             \
    static std::string_view GetTypeName();                                          \
    static Engine::FieldSpan GetFieldDescriptors();                                 \
    Engine::FieldSpan GetReflectedFields() const override;                          \
    static constexpr unsigned int StaticTypeID()                                    \
    {                                                                               \
        static const unsigned int id = Engine::DJBHash(#ClassName);                 \
        return id;                                                                  \
    }                                                                               \
                                                                                    \
    constexpr unsigned int GetTypeID() const override { return StaticTypeID(); }    \

// REFLECTED_FIELDS(ClassName, ...) — use inside the .cpp file.
// Defines GetFieldDescriptors() for ClassName with the supplied field list.
// Each entry is a FieldDescriptor braced-initialiser:
//   { "fieldName", FieldType::Vec3, offsetof(ClassName, fieldName) }
#define REFLECTED_FIELDS(ClassName, ...)                                            \
    Engine::FieldSpan ClassName::GetFieldDescriptors()                              \
    {                                                                               \
        static const Engine::FieldDescriptor fields[] { __VA_ARGS__ };              \
        return Engine::FieldSpan { &fields[0], sizeof(fields) / sizeof(fields[0])}; \
    }                                                                               \
                                                                                    \
    Engine::FieldSpan ClassName::GetReflectedFields() const                         \
    {                                                                               \
        return ClassName::GetFieldDescriptors();                                    \
    }                                                                               \

// REFLECTED_FIELDS_EMPTY(ClassName) — use when a component has no reflected
// fields but still participates in the reflection system via COMPONENT_BODY.
// TODO - needs an update
#define REFLECTED_FIELDS_EMPTY(ClassName)                               \
    Engine::FieldSpan ClassName::GetFieldDescriptors()                  \
    {                                                                   \
        return {};                                                      \
    }                                                                   \

// Register/unregister component via creation of a specific RAII type.
// !Cannot be part of the module interface (.ixx) file; place it in the
// corresponding .cpp file instead.

#define REGISTER_COMPONENT(ClassName)                                   \
    struct ClassName##_Registrar                                        \
    {                                                                   \
        ClassName##_Registrar()                                         \
        {                                                               \
            Engine::RegisterComponent(ClassName::StaticTypeID(),        \
                    []() -> std::unique_ptr<Engine::Component> {        \
                        return std::make_unique<ClassName>();           \
                    });                                                 \
        }                                                               \
        ~ClassName##_Registrar()                                        \
        {                                                               \
            Engine::UnregisterComponent(ClassName::StaticTypeID());     \
        }                                                               \
    };                                                                  \
                                                                        \
    static ClassName##_Registrar _##ClassName##_registrar{};            \
                                                                        \

// REGISTER_COMPONENT(ClassName)
// Registers the component type with an immediately invoked lambda
//#define REGISTER_COMPONENT(ClassName)                                                   \
//    inline static bool _registered_##ClassName = []() {                                 \
//        const int hash = DJBHash(#ClassName);                                           \
//        auto& registry = GetComponentFactoryRegistry();                                 \
//        if (registry.find(hash) != registry.end()) {                                    \
//            std::cerr << "Component " #ClassName " is already registered.\n";           \
//            return false;                                                               \
//        }                                                                               \
//        else                                                                            \
//        {                                                                               \
//            std::cout << "Registering component " #ClassName ".\n";                     \
//        }                                                                               \
//        registry[hash] = []() -> std::unique_ptr<Component> {                           \
//            return std::make_unique<ClassName>();                                       \
//        };                                                                              \
//        return true;                                                                    \
//    }();                                                                                \

