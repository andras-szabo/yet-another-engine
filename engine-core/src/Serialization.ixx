module;

#include <span>
#include <string>

#include "engine_core_api.h"

export module Serialization;

import DataFile;
import Math;
import Reflection;

namespace Engine
{
	export ENGINE_CORE_API
	void SerializeFields(const void* base,
						 std::span<const FieldDescriptor> fields, 
						 DataFile& out)
	{
		const char* baseAsCharPtr = reinterpret_cast<const char*>(base);
		for (const auto& field : fields)
		{
			switch (field.type)
			{
			case FieldType::Composite:
			{
				SerializeFields(baseAsCharPtr + field.offset, field.getChildren(), out[field.name]);
				break;
			}

			case FieldType::Bool:
			{
				const auto value = *reinterpret_cast<const bool*>(baseAsCharPtr + field.offset);
				out[field.name].SetInt(value ? 1 : 0);
				break;
			}
			case FieldType::Float:
			{
				const auto value = *reinterpret_cast<const float*>(baseAsCharPtr + field.offset);
				out[field.name].SetFloat(value);
				break;
			}
			case FieldType::Int:
			{
				const auto value = *reinterpret_cast<const int*>(baseAsCharPtr + field.offset);
				out[field.name].SetInt(value);
				break;
			}
			case FieldType::Quaternion:
			{
				const auto value = *reinterpret_cast<const Engine::Quaternion*>(baseAsCharPtr + field.offset);
				out[field.name].SetFloats(4, value.w, value.x, value.y, value.z);
				break;
			}
			case FieldType::String:
			{
				const auto value = *reinterpret_cast<const std::string*>(baseAsCharPtr + field.offset);
				out[field.name].SetString(value);
				break;
			}
			case FieldType::Vec2:
			{
				const auto value = *reinterpret_cast<const Vec2*>(baseAsCharPtr + field.offset);
				out[field.name].SetFloats(2, value.x, value.y);
				break;
			}
			case FieldType::Vec3:
			{
				const auto value = *reinterpret_cast<const Vec3*>(baseAsCharPtr + field.offset);
				out[field.name].SetFloats(3, value.x, value.y, value.z);
				break;
			}
			case FieldType::Vec4:
			{
				const auto value = *reinterpret_cast<const Vec4*>(baseAsCharPtr + field.offset);
				out[field.name].SetFloats(4, value.x, value.y, value.z, value.w);
				break;
			}
			}
		}
	}

	export ENGINE_CORE_API
	void DeserializeFields(void* base, std::span<const FieldDescriptor> fields, const DataFile& in)
	{
		char* baseAsCharPtr = reinterpret_cast<char*>(base);
		for (const auto& field : fields)
		{
			if (!in.HasChild(field.name))
			{
				continue;
			}

			switch (field.type)
			{
			case FieldType::Composite:
			{
				auto newBase = baseAsCharPtr + field.offset;
				DeserializeFields(newBase, field.getChildren(), in[field.name]);
				break;
			}

			case FieldType::Bool:
			{
				*reinterpret_cast<bool*>(baseAsCharPtr + field.offset) = in[field.name].GetInt() != 0;
				break;
			}
			case FieldType::Float:
			{
				*reinterpret_cast<float*>(baseAsCharPtr + field.offset) = in[field.name].GetFloat();
				break;
			}
			case FieldType::Int:
			{
				*reinterpret_cast<int*>(baseAsCharPtr + field.offset) = in[field.name].GetInt();
				break;
			}
			case FieldType::String:
			{
				*reinterpret_cast<std::string*>(baseAsCharPtr + field.offset) = in[field.name].GetString();
				break;
			}
			case FieldType::Vec2:
			{
				auto* v = reinterpret_cast<Vec2*>(baseAsCharPtr + field.offset);

				const auto df = in[field.name];
				v->x = df.GetFloat(0);
				v->y = df.GetFloat(1);

				break;
			}
			case FieldType::Vec3:
			{
				auto* v = reinterpret_cast<Vec3*>(baseAsCharPtr + field.offset);
				const auto df = in[field.name];

				v->x = df.GetFloat(0);
				v->y = df.GetFloat(1);
				v->z = df.GetFloat(2);

				break;
			}
			case FieldType::Vec4:
			{
				auto* v = reinterpret_cast<Vec4*>(baseAsCharPtr + field.offset);
				const auto df = in[field.name];

				v->x = df.GetFloat(0);
				v->y = df.GetFloat(1);
				v->z = df.GetFloat(2);
				v->w = df.GetFloat(3);

				break;
			}
			case FieldType::Quaternion:
			{
				// Stored order matches SerializeFields: w, x, y, z
				auto* q = reinterpret_cast<Quaternion*>(baseAsCharPtr + field.offset);
				const auto df = in[field.name];

				q->w = df.GetFloat(0);
				q->x = df.GetFloat(1);
				q->y = df.GetFloat(2);
				q->z = df.GetFloat(3);

				break;
			}
			}
		}
	}
} // namespace Engine
