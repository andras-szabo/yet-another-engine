module;

#include <format>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

#include "LoggerMacros.h"
#include "engine_core_api.h"

#define KEY_NODES "Nodes"
#define KEY_COMPONENTS "Components"
#define KEY_HIERARCHY "Hierarchy"

export module Serialization;

import IComponentStorage;
import DataFile;
import Logger;
import Math;
import Reflection;
import Scene;

namespace Engine
{
	export ENGINE_CORE_API
		void SerializeFields(const void* base,
			std::span<const Engine::FieldDescriptor> fields,
			Engine::DataFile& out)
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

	//const std::string KEY_HIERARCHY = "Hierarchy";
	//const std::string KEY_NODES = "Nodes";
	//const std::string KEY_COMPONENTS = "Components";


	export ENGINE_CORE_API
	Engine::Expected<Engine::Scene::Scene> DeserializeScene(const Engine::DataFile& in,
			IComponentStorage& componentStorage)
	{
		try
		{
			const auto name = in.TryGetString(0);
			if (!name.has_value())
			{
				return Engine::Unexpected(name.error());
			}

			if (!in.HasChild(KEY_HIERARCHY))
			{
				return Engine::Unexpected({ Engine::ErrorType::Deserialization, std::format("No key {} found", KEY_HIERARCHY)});
			}

			const auto& hierarchy = in[KEY_HIERARCHY];
			const auto nodeCount = hierarchy.TryGetInt(0);
			if (!nodeCount.has_value())
			{
				return Engine::Unexpected(nodeCount.error());
			}

			const int nodeCount_v = nodeCount.value();
			Engine::Scene::Scene scene{ &componentStorage, name.value(), static_cast<std::size_t>(nodeCount_v) };

			// read hierarchy info
			// read GameObject

			//Engine::TransformStorage storage{ static_cast<std::size_t>(nodeCount_v) };

			//int index = 1;
			//for (int i = 0; i < nodeCount_v; ++i)
			//{
			//	const int parent = hierarchy.GetInt(index++);
			//	const int firstChild = hierarchy.GetInt(index++);
			//	const int firstSibling = hierarchy.GetInt(index++);

			//	storage.hierarchies.emplace_back(Engine::Hierarchy { parent, firstChild, firstSibling });
			//	storage.globalTransforms.emplace_back(Engine::Mat4x4::Identity());
			//	storage.localTransforms.emplace_back(Engine::Mat4x4::Identity());
			//	storage.names.emplace_back("TBA");
			//	storage.transformComponents.emplace_back(nullptr);
			//}

			return scene;
		}
		catch (std::runtime_error e)
		{
			return Engine::Unexpected({ Engine::ErrorType::Deserialization, e.what() });
		}
	}

	export ENGINE_CORE_API
	void SerializeScene(Engine::Scene::Scene& scene, Engine::DataFile& out)
	{
		auto srlz = [&](Engine::Scene::Scene& scene_, std::size_t nodeIndex)
			{
				auto& nodes = out[KEY_NODES];
				Engine::GameObject* go = scene_.GetGameObject(nodeIndex);
				const unsigned long long guid = go->GetGUID().id;
				const std::string guidAsString = std::to_string(guid);
				const std::string goName = std::string{ go->GetName() };

				nodes[guidAsString].SetString(goName);
				Engine::DataFile& components = nodes[guidAsString][KEY_COMPONENTS];

				for (const auto& component : go->GetComponents())
				{
					const std::string typeID = std::to_string(component->GetTypeID());
					auto& componentParts = components[typeID];
					SerializeFields(component, component->GetReflectedFields(), componentParts);
				}
			};

		// 0th thing: name
		out.SetString(scene.GetSceneName());

		// 1st thing: serialize the transform storage
		auto& hierarchy = out[KEY_HIERARCHY];
		const Engine::TransformStorage* storage = scene.GetTransformStorage();

		// store only parent, firstchild, firstsibling triplets
		int index = 0;

		hierarchy.SetInt(storage->hierarchies.size(), index++);

		for (const Engine::Hierarchy& hierarchy_stored : storage->hierarchies)
		{
			hierarchy.SetInt(hierarchy_stored.parent, index++);
			hierarchy.SetInt(hierarchy_stored.firstChild, index++);
			hierarchy.SetInt(hierarchy_stored.firstSibling, index++);
		}

		scene.WalkDepthFirst(0, srlz);
	}

} // namespace Engine

