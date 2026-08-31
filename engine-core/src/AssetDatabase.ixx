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
	struct AssetDatabase_Impl
	{
		AssetDatabase_Impl()
		{
			for (const auto& [extension, type] : {
					std::make_pair(std::wstring {L".scene"}, Engine::AssetType::Scene),		// TODO
					std::make_pair(std::wstring {L".fbx"}, Engine::AssetType::StaticMesh),	// TODO
					std::make_pair(std::wstring {L".png"}, Engine::AssetType::Texture),
					// TODO
				})
			{
				assetTypesByExtension.emplace(extension, Engine::AssetType::Undefined);
			}
		}

		std::unordered_map<Engine::GUID, std::string> pathsByGuid;
		std::unordered_map<std::string, Engine::GUID> guidsByPath;
		std::unordered_map<std::wstring, Engine::AssetType> assetTypesByExtension;
	};

	export class ENGINE_CORE_API AssetDatabase
	{
	private:	
		AssetDatabase_Impl* _impl { nullptr };

	public:
		AssetDatabase();
		~AssetDatabase();

		AssetDatabase(const AssetDatabase& other) = delete;
		AssetDatabase& operator=(const AssetDatabase& other) = delete;

		AssetDatabase(AssetDatabase&& other);
		AssetDatabase& operator=(AssetDatabase&& other);

		int GetAssetCount() const;

		Engine::Expected<void> PopulateFromFolder(const std::filesystem::path& path);
		Engine::AssetType IsAssetFile(const std::filesystem::directory_entry& directoryEntry) const;

	private:
		bool DoesMatchFilter(const std::filesystem::directory_entry& directoryEntry) const;
		bool DoesMetaFileExist(const std::filesystem::directory_entry& directoryEntry, std::filesystem::path& metaFilePath) const;
		Engine::GUID CreateMetaFile(Engine::AssetType assetType, const std::filesystem::path& metaFilePath) const;
		bool TryExtractGuidAndAssetTypeFromMetaFile(const std::filesystem::path& metaFilePath, Engine::GUID& guid, Engine::AssetType& type) const;
	};

} // namespace Engine