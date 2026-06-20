module;

#include "LoggerMacros.h"
#include "engine_core_api.h"

export module Serialization;

#if defined( __INTELLISENSE__ )
#include "IComponentStorage.ixx"
#include "DataFile.ixx"
#include "EngineError.ixx"
#include "Reflection.ixx"
#include "Scene.ixx"
#else
import std;
import IComponentStorage;
import DataFile;
import Error;
import Reflection;
import Scene;
#endif

namespace Engine
{
	export ENGINE_CORE_API 
	void SerializeFields(const void* base,
			FieldSpan fieldSpan,
			Engine::DataFile& out);

	export ENGINE_CORE_API
	void DeserializeFields(void* base, FieldSpan fieldSpan, const DataFile& in);

	export ENGINE_CORE_API
	Engine::Expected<Engine::Scene::Scene> DeserializeScene(const Engine::DataFile& in,
			IComponentStorage& componentStorage);

	export ENGINE_CORE_API
	void SerializeScene(const Engine::Scene::Scene& scene, Engine::DataFile& out);
} // namespace Engine

