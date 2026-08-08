module;

#include <cassert>
#include "engine_core_api.h"

module AssetDatabase;

#if defined ( __INTELLISENSE__ )
#include "AssetDatabase.ixx"
#else
import std;
#endif

namespace Engine
{
	int AssetDatabase::GetAssetCount() const
	{
		assert(_pathsByGuid.size() == _guidsByPath.size() && "AssetDatabase internal size mismatch!");
		return _pathsByGuid.size();
	}
} // namespace Engine