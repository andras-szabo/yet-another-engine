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

	Engine::Expected<void> AssetDatabase::PopulateFromFolder(const std::filesystem::path& path)
	{
		namespace fs = std::filesystem;

		// 1. Create new std::unordered_maps
		// 2. Populate them
		//		2.1. Gather filters (e.g. partial importing!)
		//		2.2. Scan folders for all sources
		//		2.3. Create .meta files if they don't exist
		//		2.4. Check for GUID clashes
		//		2.5. If GUID clash resolved (or none), add to new maps
		// 3. If everything is OK, replace current _pathsByGuid and _guidsByPath with
		//	  the newly created ones

		// To return an error, use sg like:
		// return Engine::Unexpected{ Engine::Error { Engine::ErrorType::File, "Game DLL not found."} };

		if (!fs::exists(path))
		{
			return Engine::Unexpected{ Engine::Error { Engine::ErrorType::File, "Path not found." } };
		}

		std::unordered_map<GUID, std::string> pathsByGuid;
		std::unordered_map<std::string, GUID> guidsByPath;

		for (const auto& entry : fs::recursive_directory_iterator(path))
		{
			if (DoesMatchFilter(entry))
			{
				const auto assetType = IsAssetFile(entry);
				if (assetType != AssetType::Undefined)
				{
					// TODO
					//		- check if there's a corresponding .meta file, and create one if not
					//		- then add it to the maps
				}
			}
		}

		_pathsByGuid = std::move(pathsByGuid);
		_guidsByPath = std::move(guidsByPath);

		return {};
	}

	bool AssetDatabase::DoesMatchFilter(const std::filesystem::directory_entry& directoryEntry) const
	{
		// TODO
		return true;
	}

	AssetType AssetDatabase::IsAssetFile(const std::filesystem::directory_entry& directoryEntry) const
	{
		namespace fs = std::filesystem;

		if (directoryEntry.is_regular_file())
		{
			const std::wstring extensionAsString{ directoryEntry.path().extension() };
			const auto typeByExtensionPair = _assetTypesByExtension.find(extensionAsString);
			if (typeByExtensionPair != _assetTypesByExtension.end())
			{
				return (*typeByExtensionPair).second;
			}
		}

		return AssetType::Undefined;
	}
} // namespace Engine