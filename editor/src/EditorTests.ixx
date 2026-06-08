module;

#include <cassert>
#include <string>
#include <vector>

#include "LoggerMacros.h"

export module EditorTests;

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

namespace EditorTests
{
	export void RunSceneTest();
}

module :private;

namespace EditorTests
{
	void RunSceneTest()
	{
		auto* componentStorage = &Engine::Instance.GetComponentStorage();
		
		Engine::Scene::Scene sceneA(componentStorage, "SceneA");
		auto first = sceneA.CreateGameObject(componentStorage, "First_Child", 0);
		auto second = sceneA.CreateGameObject(componentStorage, "Second_Child",
			first->GetTransform()->GetSceneNodeIndex());
		auto third = sceneA.CreateGameObject(componentStorage, "Third_Child",
			second->GetTransform()->GetSceneNodeIndex());

		Engine::DataFile aSerialized;
		Engine::SerializeScene(sceneA, aSerialized);

		LOG_INFO("Serialized sceneA:\n{}", aSerialized.ToString());

		auto sceneMaybe = Engine::DeserializeScene(aSerialized, *componentStorage);

		assert(sceneMaybe.has_value() && "Couldn't deserialize scene.");

		auto sceneB = std::move(sceneMaybe.value());

		const auto& go_sceneA = sceneA.GetAllGameObjects();
		const auto& go_sceneB = sceneB.GetAllGameObjects();

		assert(go_sceneA.size() == go_sceneB.size() && "Size mismatch.");
		for (int i = 0; i < go_sceneA.size(); ++i)
		{
			const std::string aName{ go_sceneA[i]->GetName() };
			const std::string bName{ go_sceneB[i]->GetName() };
			assert(aName == bName && "GO name mismatch");

			const auto aGUID = go_sceneA[i]->GetGUID();
			const auto bGUID = go_sceneB[i]->GetGUID();
			assert(aGuid == bGuid && "GUID mismatch");
		}

	}
}
