module;

#include <cassert>
#include "engine_core_api.h"
#include "LoggerMacros.h"

module EngineInstance;

#if defined ( __INTELLISENSE__ )
#include "IComponentStorage.ixx"
#include "EngineInstance.ixx"
#include "Logger.ixx"
#include "ComponentStorage.ixx"
#include "Scene.ixx"
#include "Serialization.ixx"
#else
import DataFile;
import Serialization;
import Logger;
#endif

import std;

namespace Engine
{
	// Nullptr is a compile-time constant, so this counts as constant
	// initialization that should happen when the .dll is loaded, avoiding
	// the SOIF (static order of initialization fiasco).
	EngineInstance* EngineInstance::_instance = nullptr;

	Engine::Expected<std::shared_ptr<void>> CreateNewScene()
	{
		auto newScenePtr = std::make_shared<Engine::Scene::Scene>(&EngineInstance::GetComponentStorage(), "UntitledScene");

		if (!newScenePtr)
		{
			return Engine::Unexpected({ Engine::ErrorType::Undefined, "Failed to create new scene." });
		}

		return newScenePtr;
	}

	Engine::Expected<void> SaveSceneToFile(const std::string_view path, const std::shared_ptr<void> scenePtr)
	{
		if (!scenePtr)
		{
			return Engine::Unexpected({ Engine::ErrorType::Undefined, "Scene pointer is null." });
		}

		const auto scene = std::static_pointer_cast<Engine::Scene::Scene>(scenePtr);

		Engine::DataFile out;
		Engine::SerializeScene(*scene, out);

		const std::filesystem::path asPath{ path };
		const auto saveResult = Engine::DataFile::Serialize(out, asPath.string());
		if (!saveResult)
		{
			return Engine::Unexpected({ Engine::ErrorType::File, std::format("Failed to save scene to file: {}", path) });
		}
		return {};
	}

	std::shared_ptr<void> LoadSceneFromFile(const std::string_view path)
	{
		namespace fs = std::filesystem;

		// 1. Check if the scene file actually exists
		fs::path scenePath{ path };
		if (!fs::exists(scenePath))
		{
			LOG_ERROR("Scene file does not exist: {}", path);
			return nullptr;
		}

		// 2. Load the scene file into a datafile
		const auto sceneFileMaybe = Engine::DataFile::Deserialize(scenePath.string());

		if (!sceneFileMaybe.has_value())
		{
			LOG_ERROR("Failed to deserialize scene file: {}, {}", path, sceneFileMaybe.error().message);
			return nullptr;
		}

		// 3. Deserialize the scene from the datafile.
		auto sceneMaybe = Engine::DeserializeScene(sceneFileMaybe.value(),
			Engine::EngineInstance::GetComponentStorage());

		// 4. Return gloriously
		if (sceneMaybe.has_value())
		{
			return std::make_shared<Engine::Scene::Scene>(std::move(sceneMaybe.value()));
		}

		return nullptr;
	}

	struct EngineInstance_Impl
	{
		EngineInstance_Impl(std::unique_ptr<IComponentStorage> componentStorage)
			: _componentStorage(std::move(componentStorage))
		{
			// Register scene loader, creator, and saver functions with the asset db
			const auto loaderRegisterResult = _assetDatabase.RegisterAssetLoader(L".scene", LoadSceneFromFile);
			const auto saverRegisterResult = _assetDatabase.RegisterAssetSaver(L".scene", SaveSceneToFile);
			const auto creatorRegisterResult = _assetDatabase.RegisterAssetCreator(L".scene", CreateNewScene);

			if (!loaderRegisterResult.has_value())
			{
				LOG_ERROR("Failed to register scene asset loader: {}", loaderRegisterResult.error().message);
			}
			else
			{
				LOG_INFO("Scene asset loader registered successfully.");
			}

			if (!saverRegisterResult.has_value())
			{
				LOG_ERROR("Failed to register scene asset saver: {}", saverRegisterResult.error().message);
			}
			else
			{
				LOG_INFO("Scene asset saver registered successfully.");
			}

			if (!creatorRegisterResult.has_value())
			{
				LOG_ERROR("Failed to register scene asset creator: {}", creatorRegisterResult.error().message);
			}
			else
			{
				LOG_INFO("Scene asset creator registered successfully.");
			}

			//_activeScene = new Engine::Scene::Scene(_componentStorage.get(), "UntitledScene");
		}

		~EngineInstance_Impl()
		{
			_activeScene = nullptr;
		}

		void SetActiveScene(Scene::Scene && scene)
		{
			_activeScene = std::make_shared<Engine::Scene::Scene>(std::forward<Scene::Scene&&>(scene));
		}

		void SetActiveScene(std::shared_ptr<Scene::Scene> scenePtr)
		{
			_activeScene = scenePtr;
		}

		void Run();
		void Update(double dt);
		void OnPlayModeStart();
		void OnPlayModeStop();
		void OnApplicationStart();
		void OnApplicationQuit();

		bool IsQuitting() const;
		float CalculateDeltaTime() const;

		std::unique_ptr<IComponentStorage> _componentStorage;
		std::shared_ptr<Scene::Scene> _activeScene{ nullptr };
		Engine::AssetDatabase _assetDatabase;

		double _elapsedSecondsSinceStarup{ 0.0 };
		double _elapsedSecondsInPlayMode{ 0.0 };
		double _elapsedSinceLastFixedUpdate{ 0.0 };
		double _fixedUpdateStep{ 1.0 / 60.0 };

		std::chrono::high_resolution_clock::time_point _applicationStartTime;
	};

	// static
	void EngineInstance::Initialize()
	{
		if (_instance == nullptr)
		{
			_instance = new EngineInstance(std::make_unique<ComponentStorage>());
		}
	}

	// static
	void EngineInstance::Initialize(std::unique_ptr<IComponentStorage> componentStorage)
	{
		if (_instance == nullptr)
		{
			_instance = new EngineInstance(std::move(componentStorage));
		}
	}

	// static
	Engine::Expected<void> EngineInstance::Shutdown()
	{
		try
		{
			delete _instance;
			_instance = nullptr;
			return {};
		}
		catch (const std::exception& e)
		{
			return Engine::Unexpected({ Engine::ErrorType::Undefined, {"Failed to shutdown EngineInstance: {}", e.what()}});
		}
	}

	// static
	EngineInstance& EngineInstance::Get()
	{
		return *_instance;
	}

	// static
	AssetDatabase& EngineInstance::GetAssetDatabase()
	{
		assert(_instance != nullptr && "Engine instance not initialized?");
		return _instance->_impl->_assetDatabase;
	}

	// static
	IComponentStorage& EngineInstance::GetComponentStorage()
	{
		assert(_instance != nullptr && "Engine instance not initialized?");
		return *(_instance->_impl->_componentStorage.get());
	}

	// static
	const Scene::Scene& EngineInstance::GetActiveScene()
	{
		assert(_instance != nullptr && "Engine instance not initialized?");
		assert(_instance->_impl->_activeScene != nullptr && "Active scene is not set.");

		return *(_instance->_impl->_activeScene);
	}

	// static
	Scene::Scene& EngineInstance::GetActiveSceneRW()
	{
		assert(_instance != nullptr && "Engine instance not initialized?");
		assert(_instance->_impl->_activeScene != nullptr && "Active scene is not set.");

		return *(_instance->_impl->_activeScene);
	}

	// static
	bool EngineInstance::HasActiveScene()
	{
		return _instance != nullptr && _instance->_impl->_activeScene != nullptr;
	}


	// static
	void EngineInstance::SetActiveScene(Scene::Scene&& scene)
	{
		assert(_instance != nullptr && "Engine instance is not initialized!");
		_instance->_impl->SetActiveScene(std::forward<Scene::Scene&&>(scene));
	}

	// static
	void EngineInstance::SetActiveScene(std::shared_ptr<Scene::Scene> scenePtr)
	{
		assert(_instance != nullptr && "Engine instance is not initialized!");
		_instance->_impl->SetActiveScene(scenePtr);
	}

	EngineInstance::~EngineInstance()
	{
		delete _impl;
		_impl = nullptr;
	}

	EngineInstance::EngineInstance(std::unique_ptr<IComponentStorage> componentStorage)
	{
		_impl = new EngineInstance_Impl(std::forward<std::unique_ptr<IComponentStorage>>(componentStorage));
	}

	void EngineInstance::Run()
	{
		assert(_impl != nullptr && "Engine instance is not initialized!");
		_impl->Run();
	}

	void EngineInstance_Impl::Run()
	{
		while (!IsQuitting())
		{
			float dt = CalculateDeltaTime();
			Update(dt);
		}
	}

	bool EngineInstance_Impl::IsQuitting() const
	{
		// TODO
		return false;
	}

	void EngineInstance::Update(double dt)
	{
		assert(_impl != nullptr && "Engine instance is not initialized!");
		_impl->Update(dt);
	}

	void EngineInstance::OnApplicationStart()
	{
		assert(_impl != nullptr && "Engine instance is not initialized!");
		LOG_INFO("On Application Start");
		_impl->OnApplicationStart();
	}

	void EngineInstance::OnApplicationQuit()
	{
		assert(_impl != nullptr && "Engine instance is not initialized!");
		LOG_INFO("On Application Quit");
		_impl->OnApplicationQuit();
	}

	void EngineInstance::OnPlayModeStart()
	{
		assert(_impl != nullptr && "Engine instance is not initialized!");
		LOG_INFO("On Play Mode Start");
		_impl->OnPlayModeStart();
	}

	void EngineInstance::OnPlayModeStop()
	{
		assert(_impl != nullptr && "Engine instance is not initialized!");
		LOG_INFO("On Play Mode Stop");
		_impl->OnPlayModeStop();
	}

	// TODO revise
	float EngineInstance_Impl::CalculateDeltaTime() const
	{
		static std::chrono::high_resolution_clock clock;
		static auto lastTime = clock.now();

		auto currentTime = clock.now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;

		return deltaTime;
	}

	void EngineInstance_Impl::OnPlayModeStart()
	{
		_elapsedSecondsInPlayMode = 0.0;
	}

	void EngineInstance_Impl::OnPlayModeStop()
	{
	}

	void EngineInstance_Impl::OnApplicationStart()
	{
		_applicationStartTime = std::chrono::high_resolution_clock::now();
	}

	void EngineInstance_Impl::OnApplicationQuit()
	{
	}

	void EngineInstance_Impl::Update(double dt)
	{
		const long double elapsedSeconds = trunc(static_cast<long double>(_elapsedSecondsInPlayMode));
		_elapsedSecondsInPlayMode += dt;

		// Update input
		// Update engine systems (physics etc)
		
		// Fixed update steps
		_elapsedSinceLastFixedUpdate += dt;
		while (_elapsedSinceLastFixedUpdate >= _fixedUpdateStep)
		{
			_elapsedSinceLastFixedUpdate -= _fixedUpdateStep;
			if (_activeScene != nullptr)
			{
				_activeScene->UpdateWorldTransforms();
				for (auto& gameObject : _activeScene->GetAllGameObjects())
				{
					for (auto* component : gameObject->GetComponents())
					{
						if (component->IsEnabled())
						{
							component->OnFixedUpdate(static_cast<double>(_fixedUpdateStep));
						}
					}
				}
			}
		}

		// Variable rate update step
		if (_activeScene != nullptr)
		{
			_activeScene->UpdateWorldTransforms();

			for (auto& gameObject : _activeScene->GetAllGameObjects())
			{
				for (auto* component : gameObject->GetComponents())
				{
					if (component->IsEnabled())
					{
						component->OnUpdate(dt);
					}
				}
			}
		}

		{
			const long double seconds = trunc(static_cast<long double>(_elapsedSecondsInPlayMode));

			if (seconds > elapsedSeconds)
			{
				LOG_INFO("EngineInstance update: {} seconds elapsed, dt = {}", seconds, dt);
			}
		}
	}

} // namespace Engine