#include "GameDLLInterface.h"

#include "ReflectionMacros.h"
#include "LoggerMacros.h"

#if defined ( __INTELLISENSE__ )
#include "../../engine-core/src/EngineCore.ixx"
#include "../../engine-core/src/EngineInstance.ixx"
#else
import EngineCore;
import EngineInstance;
import SpinnerComponent;
#endif

unsigned int GameDLL_GetVersion()
{
	return GAME_DLL_ABI_VERSION;
}
