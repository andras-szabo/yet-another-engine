#pragma once

#define GAME_DLL_ABI_VERSION 1

extern "C"
{
	// Returns GAME_DELL_ABI_VERSION baked into the DLL at compile time.
	// Editor rejects a DLL whose version doesn't match what it expects.
	__declspec(dllexport) unsigned int GameDLL_GetVersion();

	// Called immediately after load.
}
