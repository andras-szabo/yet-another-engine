module;

#include <cassert>
#include "engine_core_api.h"
#include "LoggerMacros.h"

module EngineInstance;

#if defined ( __INTELLISENSE__ )
#include "IComponentStorage.ixx"
#include "EngineInstance.ixx"
#include "Logger.ixx"
#include "Scene.ixx"
#else
import IComponentStorage;
import Scene;
import Logger;
#endif

import std;

namespace Engine
{
	// Nullptr is a compile-time constant, so this counts as constant
	// initialization that should happen when the .dll is loaded, avoiding
	// the SOIF.
	EngineInstance* EngineInstance::_instance = nullptr;

	// static
	void EngineInstance::Initialize(std::unique_ptr<IComponentStorage> componentStorage)
	{
		if (_instance == nullptr)
		{
			_instance = new EngineInstance(std::move(componentStorage));
		}
	}

	// static
	void EngineInstance::Shutdown()
	{
		delete _instance;
		_instance = nullptr;
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
		return *(_instance->_impl->_activeScene);
	}

	// static
	Scene::Scene& EngineInstance::GetActiveSceneRW()
	{
		assert(_instance != nullptr && "Engine instance not initialized?");
		return *(_instance->_impl->_activeScene);
	}

	// static
	void EngineInstance::SetActiveScene(Scene::Scene&& scene)
	{
		assert(_instance != nullptr && "Engine instance is not initialized!");
		_instance->_impl->SetActiveScene(std::forward<Scene::Scene&&>(scene));
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

	void EngineInstance::Update(float dt)
	{
		assert(_impl != nullptr && "Engine instance is not initialized!");
		_impl->Update(dt);
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

	void EngineInstance_Impl::Update(float dt)
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
							component->OnFixedUpdate(static_cast<float>(_fixedUpdateStep));
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