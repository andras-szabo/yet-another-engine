#pragma once

#include <cstddef>      // offsetof
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
//   #include "MyComponent.reflected.h"   // <-- after the class definition
//
// ---- MyComponent.reflected.h (hand-written; tool-generated in the future) ----
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
#define COMPONENT_BODY(ClassName)                                                      \
public:                                                                                \
    static constexpr std::string_view TypeName { #ClassName };                        \
    static std::string_view GetTypeName() { return TypeName; }                        \
    static std::span<const FieldDescriptor> GetFieldDescriptors();                    \
    std::span<const FieldDescriptor> GetReflectedFields() const override               \
    { return ClassName::GetFieldDescriptors(); }

// REFLECTED_FIELDS(ClassName, ...) — use inside a ClassName.reflected.h file.
// Defines GetFieldDescriptors() for ClassName with the supplied field list.
// Each entry is a FieldDescriptor braced-initialiser:
//   { "fieldName", FieldType::Vec3, offsetof(ClassName, fieldName) }
#define REFLECTED_FIELDS(ClassName, ...)                                               \
    std::span<const FieldDescriptor> ClassName::GetFieldDescriptors()                  \
    {                                                                                  \
        static const FieldDescriptor fields[] { __VA_ARGS__ };                        \
        return fields;                                                                 \
    }

// REFLECTED_FIELDS_EMPTY(ClassName) — use when a component has no reflected
// fields but still participates in the reflection system via COMPONENT_BODY.
#define REFLECTED_FIELDS_EMPTY(ClassName)                                              \
    std::span<const FieldDescriptor> ClassName::GetFieldDescriptors()                  \
    {                                                                                  \
        return {};                                                                     \
    }
