#include "GameDLLInterface.h"

#include "../../engine-core/include/ReflectionMacros.h"
#include "../../engine-core/include/LoggerMacros.h"

#if defined ( __INTELLISENSE__ )
#include "../../engine-core/src/EngineCore.ixx"
#else
import EngineCore;
import EngineInstance;

// Note: unless consoming the STL as a named module, we can run into weird
// linker errors as soon as trying to use logger macros.
//import std;
#endif

unsigned int GameDLL_GetVersion()
{
	return GAME_DLL_ABI_VERSION;
}

// Here until I find a better place for it:
class MyComponent : public Engine::Component
{
	COMPONENT_BODY(MyComponent)
	REGISTER_COMPONENT(MyComponent)

public:
	MyComponent() = default;
	~MyComponent() override = default;
};

REFLECTED_FIELDS_EMPTY(MyComponent)

//int ThisIsWhatMainUsedToBe()
//{
//	LOG_INFO("[game-template] stub");
//
//	return 0;
//}
