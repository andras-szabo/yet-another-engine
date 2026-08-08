module;

#include "engine_core_api.h"

export module AssetDatabase;

#if defined ( __INTELLISENSE__ )
#include <unordered_map>

#include "Asset.ixx"
#include "Guid.ixx"

#else
import Asset;
import GUID;

import std;
#endif

namespace Engine
{
	export class ENGINE_CORE_API AssetDatabase
	{
	private:	
		std::unordered_map<Engine::GUID, std::string> _pathsByGuid;
		std::unordered_map<std::string, Engine::GUID> _guidsByPath;

	public:
		int GetAssetCount() const;
	};

} // namespace Engine