module;

#include "engine_core_api.h"

export module AssetDatabase;

#if defined ( __INTELLISENSE__ )
#include <filesystem>
#include <unordered_map>

#include "Asset.ixx"
#include "EngineError.ixx"
#include "Guid.ixx"

#else
import Asset;
import GUID;
import Error;
import std;
#endif

namespace Engine
{
	export class ENGINE_CORE_API AssetDatabase
	{
	private:	
		std::unordered_map<Engine::GUID, std::string> _pathsByGuid;
		std::unordered_map<std::string, Engine::GUID> _guidsByPath;

		std::unordered_map<std::wstring, Engine::AssetType> _assetTypesByExtension;

	public:
		int GetAssetCount() const;

		Engine::Expected<void> PopulateFromFolder(const std::filesystem::path& path);
		Engine::AssetType IsAssetFile(const std::filesystem::directory_entry& directoryEntry) const;

	private:
		bool DoesMatchFilter(const std::filesystem::directory_entry& directoryEntry) const;
	};

} // namespace Engine