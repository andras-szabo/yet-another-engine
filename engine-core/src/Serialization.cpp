module;

#include <cassert>
#include "engine_core_api.h"
#include "LoggerMacros.h"

module Serialization;

#if defined( __INTELLISENSE__ )
#include "ComponentRegistry.ixx"
#include "IComponentStorage.ixx"
#include "DataFile.ixx"
#include "GameObject.ixx"
#include "GUID.ixx"
#include "Logger.ixx"
#include "Math.ixx"
#include "Reflection.ixx"
#include "Scene.ixx"
#include "Transform.ixx"
#else
import ComponentRegistry;
import IComponentStorage;
import DataFile;
import Error;
import GUID;
import Logger;
import Math;
import Reflection;
import Scene;
import Transform;
#endif

import std;

#define KEY_NODES "Nodes"
#define KEY_COMPONENTS "Components"
#define KEY_HIERARCHY "Hierarchy"

namespace Engine
{
	Engine::Expected<void> SetSerializedFieldFromString(std::string_view fieldName, 
		std::string_view valueAsString, 
		void* base, 
		const FieldSpan fieldSpan)
	{
		std::span<const FieldDescriptor> fields(fieldSpan.data, fieldSpan.size);
		char* baseAsCharPtr = reinterpret_cast<char*>(base);
		for (const auto& field : fields)
		{
			if (field.name == fieldName)
			{
				switch (field.type)
				{
				case FieldType::Bool:
				{
					bool value = (valueAsString == "true" || valueAsString == "1");
					*reinterpret_cast<bool*>(baseAsCharPtr + field.offset) = value;
					return {};
				}
				case FieldType::Float:
				{
					float value = std::stof(std::string{ valueAsString });
					*reinterpret_cast<float*>(baseAsCharPtr + field.offset) = value;
					return {};
				}
				case FieldType::Int:
				{
					int value = std::stoi(std::string{ valueAsString });
					*reinterpret_cast<int*>(baseAsCharPtr + field.offset) = value;
					return {};
				}
				case FieldType::String:
				{
					std::string value{ valueAsString };
					*reinterpret_cast<std::string*>(baseAsCharPtr + field.offset) = std::move(value);
					return {};
				}
				case FieldType::Vec3:
				{
					Vec3 value;
					if (!Vec3::Parse(valueAsString, value))
					{
						return Engine::Unexpected({ Engine::ErrorType::Serialization, std::format("Failed to parse Vec3 for field {}", fieldName) });
					}
					*reinterpret_cast<Vec3*>(baseAsCharPtr + field.offset) = value;
					return {};
				}
				default:
					return Engine::Unexpected({ Engine::ErrorType::Serialization, std::format("Unsupported field type for field {}", fieldName) });
				}
			}
			else if (field.type == FieldType::Composite && field.getChildren != nullptr)
			{
				void* newBase = baseAsCharPtr + field.offset;
				return SetSerializedFieldFromString(fieldName, valueAsString, newBase, field.getChildren());
			}
		}

		return Engine::Unexpected({ Engine::ErrorType::Serialization, std::format("Field '{}' not found", fieldName) });
	}

	void SerializeFields(const void* base,
		const FieldSpan fieldSpan,
		Engine::DataFile& out)
	{
		const std::span<const FieldDescriptor> fields(fieldSpan.data, fieldSpan.size);
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
			case FieldType::GUID:
			{
				const auto value = *reinterpret_cast<const GUID*>(baseAsCharPtr + field.offset);
				out[field.name].SetULong(static_cast<unsigned long long>(value));
				break;
			}
			}
		}
	}

	void DeserializeFields(void* base, const FieldSpan fieldSpan, const DataFile& in)
	{
		std::span<const FieldDescriptor> fields(fieldSpan.data, fieldSpan.size);
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

				const auto& df = in[field.name];
				v->x = df.GetFloat(0);
				v->y = df.GetFloat(1);

				break;
			}
			case FieldType::Vec3:
			{
				auto* v = reinterpret_cast<Vec3*>(baseAsCharPtr + field.offset);
				const auto& df = in[field.name];

				v->x = df.GetFloat(0);
				v->y = df.GetFloat(1);
				v->z = df.GetFloat(2);

				break;
			}
			case FieldType::Vec4:
			{
				auto* v = reinterpret_cast<Vec4*>(baseAsCharPtr + field.offset);
				const auto& df = in[field.name];

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
				const auto& df = in[field.name];

				q->w = df.GetFloat(0);
				q->x = df.GetFloat(1);
				q->y = df.GetFloat(2);
				q->z = df.GetFloat(3);

				break;
			}
			case FieldType::GUID:
			{
				auto* g = reinterpret_cast<GUID*>(baseAsCharPtr + field.offset);

				const auto& df = in[field.name];
				*g = GUID(df.GetULong(0));

				break;
			}
			}
		}
	}

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
				return Engine::Unexpected({ Engine::ErrorType::Deserialization, std::format("No key {} found", KEY_HIERARCHY) });
			}

			const auto& hierarchy = in[KEY_HIERARCHY];
			const auto nodeCount = hierarchy.TryGetInt(0);
			if (!nodeCount.has_value())
			{
				return Engine::Unexpected(nodeCount.error());
			}

			const int nodeCount_v = nodeCount.value();
			std::vector<Engine::Hierarchy> hierarchies;
			hierarchies.reserve(nodeCount_v);

			if (nodeCount_v <= 0)
			{
				return Engine::Unexpected({ Engine::ErrorType::Deserialization, "No nodes found; at least one root node expected." });
			}

			int index = 1;
			LOG_INFO("Deserializing scene: {} with {} nodes", name.value(), nodeCount_v);
			for (int i = 0; i < nodeCount_v; ++i)
			{
				const int parent = hierarchy.GetInt(index++);
				const int firstChild = hierarchy.GetInt(index++);
				const int nextSibling = hierarchy.GetInt(index++);

				// prevSibling isn't stored; the links are rebuilt by AddTransform on load.
				hierarchies.emplace_back(Engine::Hierarchy{ parent, firstChild, nextSibling });
			}

			if (!in.HasChild(KEY_NODES))
			{
				return Engine::Unexpected({ Engine::ErrorType::Deserialization, std::format("No key {} found", KEY_NODES) });
			}

			const auto& nodes = in[KEY_NODES];
			const auto rootGuidAsString = nodes.GetChildrenNames()[0];
			const auto rootGuid = std::stoull(rootGuidAsString);

			Engine::Scene::Scene scene{ &componentStorage,
				name.value(),
				static_cast<std::size_t>(nodeCount_v),
				rootGuid };

			std::size_t nodeIndex = 0;
			for (const std::string& guidStr : nodes.GetChildrenNames())
			{
				// The root should already have been saved
				LOG_INFO(guidStr);
				if (nodeIndex > 0)
				{
					const auto _guid = std::stoull(guidStr);		// The node (gameObject) guid
					const auto gameObjectName = nodes[guidStr].GetString();
					const auto parentNodeIndex = hierarchies[nodeIndex].parent;
					LOG_INFO("Deserializing game object: {} with GUID: {} at parent node index: {}", gameObjectName, _guid, parentNodeIndex);
					auto* go = scene.CreateGameObject(&componentStorage,
						gameObjectName,
						parentNodeIndex,
						_guid);

					const auto& componentTypeIDs = nodes[guidStr][KEY_COMPONENTS].GetChildrenNames();

					for (const auto& componentTypeID : componentTypeIDs)
					{
						const unsigned int typeID = std::stoul(componentTypeID);
						Component* component{ nullptr };
						if (typeID == Transform::StaticTypeID())
						{
							component = static_cast<Component*>(go->GetTransform());
						}
						else
						{
							auto componentMaybe = GlobalComponentRegistry().Create(typeID, componentStorage);
							if (!componentMaybe.has_value())
							{
								LOG_WARNING("Component read fail: {}", componentMaybe.error().message);
								continue;
							}

							component = componentMaybe.value();
							go->AddComponentRaw(component);
						}

						assert(component != nullptr);

						const auto& componentNode = nodes[guidStr][KEY_COMPONENTS][componentTypeID];
						DeserializeFields(component, component->GetReflectedFields(), componentNode);
						component->OnCreate();
					}

				}

				nodeIndex++;
			}

			return scene;
		}
		catch (const std::runtime_error& e)
		{
			return Engine::Unexpected({ Engine::ErrorType::Deserialization, e.what() });
		}
	}

	// TODO handle errors
	void SerializeScene(const Engine::Scene::Scene& scene, Engine::DataFile& out)
	{
		// 0th thing: name
		out.SetString(scene.GetSceneName());

		// 1st thing: serialize the transform storage
		auto& hierarchy = out[KEY_HIERARCHY];
		const Engine::TransformStorage* storage = scene.GetTransformStorage();

		// Node indices in memory are not stable (removal is swap-and-pop), and a parent may have a
		// higher index than its child. The file format, however, requires parents to come before
		// their children, so we write the nodes in depth-first preorder and remap the indices.
		const int nodeCount = static_cast<int>(storage->hierarchies.size());
		std::vector<int> fileOrder;		// file position -> in-memory index
		fileOrder.reserve(nodeCount);
		std::vector<int> fileIndexOf(nodeCount, -1);	// in-memory index -> file position

		{
			const auto& hs = storage->hierarchies;
			int node = nodeCount > 0 ? 0 : -1;
			while (node != -1)
			{
				fileIndexOf[node] = static_cast<int>(fileOrder.size());
				fileOrder.push_back(node);

				if (hs[node].firstChild != -1)
				{
					node = hs[node].firstChild;
				}
				else
				{
					while (node != -1 && hs[node].nextSibling == -1)
					{
						node = hs[node].parent;
					}
					if (node != -1)
					{
						node = hs[node].nextSibling;
					}
				}
			}
		}

		auto remap = [&](int i) { return i == -1 ? -1 : fileIndexOf[i]; };

		// store only parent, firstchild, nextsibling triplets
		int index = 0;

		hierarchy.SetInt(nodeCount, index++);	// node count

		for (const int memIndex : fileOrder)
		{
			const Engine::Hierarchy& hierarchy_stored = storage->hierarchies[memIndex];
			hierarchy.SetInt(remap(hierarchy_stored.parent), index++);
			hierarchy.SetInt(remap(hierarchy_stored.firstChild), index++);
			hierarchy.SetInt(remap(hierarchy_stored.nextSibling), index++);
		}

		auto& nodes = out[KEY_NODES];

		const std::vector<std::unique_ptr<GameObject>>& nodePtrs = scene.GetAllGameObjects();
		for (const int memIndex : fileOrder)
		{
			const GameObject* go = nodePtrs[memIndex].get();
			const unsigned long long _guid = go->GetGUID().id;
			const std::string guidAsString = std::to_string(_guid);
			const std::string goName = std::string{ go->GetName() };

			nodes[guidAsString].SetString(goName);
			Engine::DataFile& components = nodes[guidAsString][KEY_COMPONENTS];

			for (const auto& component : go->GetComponents())
			{
				const std::string typeID = std::to_string(component->GetTypeID());
				auto& componentParts = components[typeID];
				SerializeFields(component, component->GetReflectedFields(), componentParts);
			}
		}
	}
} // namespace Engine