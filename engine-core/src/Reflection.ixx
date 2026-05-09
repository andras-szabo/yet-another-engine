module;

#include <cstddef>
#include <span>
#include <string_view>

export module Reflection;

namespace Engine
{
    // FieldType — tags the C++ type of a reflected field.
    // Extend this enum as new serialisable types are needed.
    export
        enum class FieldType
    {
        Float,
        Int,
        Bool,
        String,
        Vec2,
        Vec3,
        Vec4,
        Quaternion,
    };

    // FieldDescriptor — describes one reflected field on a Component subclass.
    //
    // 'offset' is the byte offset of the field from the start of the object,
    // obtained via offsetof() in the corresponding .reflected.h file.
    // The serialiser uses it to read/write the field on any live instance.
    export
        struct FieldDescriptor
    {
        std::string_view name;
        FieldType        type;
        std::size_t      offset;
    };
}
