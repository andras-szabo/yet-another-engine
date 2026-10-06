module;

#include "engine_core_api.h"

export module AssetDatabase;

#if defined ( __INTELLISENSE__ )
#include <filesystem>
#include <span>
#include <unordered_map>
#include "Asset.ixx"
#include "EngineError.ixx"
#include "FileWatcher.ixx"
#include "Guid.ixx"
#else
import Asset;
import GUID;
import Error;
import FileWatcher;
import std;
#endif

namespace Engine
{
	/// <summary>
	/// A function that, given a path, reads the file, and turns it into some kind of
	/// deserialized blob of data, which you'll be able to cast to a specific type of
	/// asset with static_pointer_cast.
	/// </summary>
	export using AssetLoaderFn = std::shared_ptr<void>(*)(const std::string_view path);

	struct AssetDatabase_Impl
	{
		AssetDatabase_Impl()
		{
			for (const auto& [extension, type] : {
					std::make_pair(std::wstring {L".scene"}, Engine::AssetType::Scene),		// TODO
					std::make_pair(std::wstring {L".fbx"}, Engine::AssetType::StaticMesh),	// TODO
					std::make_pair(std::wstring {L".png"}, Engine::AssetType::Texture),
					std::make_pair(std::wstring {L".jpg"}, Engine::AssetType::Texture),
					// TODO
				})
			{
				assetTypesByExtension.emplace(extension, type);
			}
		}

		std::unordered_map<Engine::GUID, std::string> pathsByGuid;
		std::unordered_map<std::string, Engine::GUID> guidsByPath;
		std::unordered_map<std::wstring, Engine::AssetType> assetTypesByExtension;

		Engine::Expected<void> RegisterAssetLoader(const std::wstring& extension, AssetLoaderFn loaderFn);
		Engine::Expected<std::shared_ptr<void>> LoadAsset(const Engine::GUID guid);
		Engine::Expected<std::shared_ptr<void>> LoadAsset(const std::string_view path);

		bool IsAssetLoaded(const Engine::GUID guid) const;
		bool ReleaseAsset(const Engine::GUID guid);

	private:
		std::unordered_map<Engine::GUID, std::weak_ptr<void>> _loadedAssetsByGUID;
		std::unordered_map<std::wstring, AssetLoaderFn> _loaderFunctionsByExtension;

		Engine::Expected<std::shared_ptr<void>> DoLoadAssetFromPath(const std::string_view path);
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

		void Update(std::span<const Engine::FileChangeEvent> fileChangeEvents);

		template <typename T>
		bool IsAssetLoaded(const AssetRef<T>& assetRef) const;

		template <typename T>
		Engine::Expected<T*> LoadAsset(AssetRef<T>& assetRef);

		template <typename T>
		bool ReleaseAsset(AssetRef<T>& assetRef);

	private:
		bool DoesMatchFilter(const std::filesystem::directory_entry& directoryEntry) const;
		bool DoesMetaFileExist(const std::filesystem::directory_entry& directoryEntry, std::filesystem::path& metaFilePath) const;
		Engine::GUID CreateMetaFile(Engine::AssetType assetType, const std::filesystem::path& metaFilePath) const;
		bool TryExtractGuidAndAssetTypeFromMetaFile(const std::filesystem::path& metaFilePath, Engine::GUID& _guid, Engine::AssetType& type) const;
		std::string AssetTypeToString(Engine::AssetType assetType) const;
	};

	template <typename T>
	bool AssetDatabase::IsAssetLoaded(const AssetRef<T>& assetRef) const
	{
		return _impl->IsAssetLoaded(assetRef.Guid());
	}

	template <typename T>
	Engine::Expected<T*> AssetDatabase::LoadAsset(AssetRef<T>& assetRef)
	{
		Expected<std::shared_ptr<void>> loadedAssetPtr = _impl->LoadAsset(assetRef.Guid());
		if (loadedAssetPtr.has_value())
		{
			assetRef.Assign(std::static_pointer_cast<T>(loadedAssetPtr.value()));
			return assetRef.Ptr();
		}

		return loadedAssetPtr.error();
	}

	/// <summary>
	/// Releases the asset held by the given AssetRef. This will decrement the ref count of the asset,
	/// and if the ref count reaches zero, the asset will be unloaded from memory. Either way, the 
	/// AssetRef's pointer will be released and set to nullptr.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="assetRef"></param>
	/// <returns>Whether the asset was unloaded from memory as the result of this release.</returns>
	template <typename T>
	bool AssetDatabase::ReleaseAsset(AssetRef<T>& assetRef)
	{
		if (assetRef.Ptr())
		{
			assetRef.Assign(nullptr);
			return _impl->ReleaseAsset(assetRef.Guid());
		}

		return false;
	}

} // namespace Engine